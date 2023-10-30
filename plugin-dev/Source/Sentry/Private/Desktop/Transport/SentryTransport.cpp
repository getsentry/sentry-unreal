// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryTransport.h"

#include "SentryDefines.h"
#include "SentryModule.h"
#include "SentryDsnUrl.h"
#include "SentrySettings.h"

#include "Desktop/Infrastructure/SentryConvertorsDesktop.h"

#include "HttpModule.h"
#include "HttpManager.h"
#include "Misc/ScopeLock.h"

#if USE_SENTRY_NATIVE

sentry_transport_t* FSentryTransport::Create()
{
	auto NewTransport = MakeShared<FSentryTransport, ESPMode::ThreadSafe>();

	// Initialize transport object with the corresponding self-reference in order to control its lifetime from within `sentry-native`
	NewTransport->Self = NewTransport;

	sentry_transport_t* transport = sentry_transport_new([](sentry_envelope_t* envelope, void* state)
	{
		static_cast<FSentryTransport*>(state)->Send(envelope);
	});

	sentry_transport_set_state(transport, &NewTransport.Get());

	sentry_transport_set_free_func(transport, [](void* state)
	{
		static_cast<FSentryTransport*>(state)->Free();
	});
	sentry_transport_set_startup_func(transport, [](const sentry_options_t* options, void* state)->int
	{
		return static_cast<FSentryTransport*>(state)->Startup(options);
	});
	sentry_transport_set_flush_func(transport, [](uint64_t timeout, void* state)->int
	{
		return static_cast<FSentryTransport*>(state)->Flush(timeout);
	});
	sentry_transport_set_shutdown_func(transport, [](uint64_t timeout, void* state)->int
	{
		return static_cast<FSentryTransport*>(state)->Shutdown(timeout);
	});

	return transport;
}

void FSentryTransport::Send(sentry_envelope_t* envelope)
{
	if(!IsRunning)
	{
		return;
	}

	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	const SentryDsnUrl DsnUrl(Settings->Dsn);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(FString::Printf(TEXT("https://%s/api/%s/envelope/"), *DsnUrl.GetHost(), *DsnUrl.GetProjectId()));
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/x-sentry-envelope"));
	HttpRequest->SetHeader(TEXT("X-Sentry-Auth"), FString::Printf(TEXT("Sentry sentry_version=7, sentry_client=sentry-unreal, sentry_key=%s"), *DsnUrl.GetKey()));
	HttpRequest->SetContent(SentryConvertorsDesktop::SentryEnvelopeToByteArray(envelope));

	HttpRequest->OnProcessRequestComplete().BindThreadSafeSP(this, &FSentryTransport::OnRequestCompleted);

	{
		FScopeLock Lock(&CriticalSection);
		RequestsQueue.Add(HttpRequest);
		if (!HttpRequest->ProcessRequest())
		{
			RequestsQueue.Pop();
		}
	}

	sentry_envelope_free(envelope);
}

void FSentryTransport::Free()
{
	Self.Reset();
}

int FSentryTransport::Startup(const sentry_options_t* options)
{
	IsRunning = true;
	return 0;
}

int FSentryTransport::Flush(uint64_t timeout)
{
	const float InTimeOut = static_cast<double>(timeout) / 1000;
	const float SleepInterval = 0.01f;

	float TimeElapsed = 0.0f;

	while(true)
	{
		if(RequestsQueue.Num() == 0)
		{
			break;
		}

		// Manually tick pending requests since relying on game thread for that might be insecure
		FHttpModule::Get().GetHttpManager().Tick(SleepInterval);

		if(RequestsQueue.Num() > 0)
		{
			if (TimeElapsed > InTimeOut)
			{
				return 1;
			}

			FPlatformProcess::Sleep(SleepInterval);
		}

		TimeElapsed += SleepInterval;
	}

	return 0;
}

int FSentryTransport::Shutdown(uint64_t timeout)
{
	IsRunning = false;
	return Flush(timeout);
}

void FSentryTransport::OnRequestCompleted(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request, TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response, bool bSuccess)
{
	if(!bSuccess || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Sending envelope failed."));
	}

	FScopeLock Lock(&CriticalSection);

	int32 RequestIndex = RequestsQueue.IndexOfByPredicate([&](const TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>& CachedRequest)
	{
		return CachedRequest.Get() == Request.Get();
	});

	if(RequestIndex != INDEX_NONE)
	{
		RequestsQueue.RemoveAt(RequestIndex);
	}
}

#endif