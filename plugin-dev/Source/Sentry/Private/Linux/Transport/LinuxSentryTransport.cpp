// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryTransport.h"

#include "SentryDefines.h"
#include "SentryModule.h"
#include "Core/SentryDsnUrl.h"
#include "SentrySettings.h"

#include "GenericPlatform/Infrastructure/GenericPlatformSentryConverters.h"

#include "HttpModule.h"
#include "HttpManager.h"
#include "Misc/ScopeLock.h"

#if USE_SENTRY_NATIVE

sentry_transport_t* FLinuxSentryTransport::Create()
{
	auto NewTransport = MakeShared<FLinuxSentryTransport, ESPMode::ThreadSafe>();

	// Initialize transport object with the corresponding self-reference in order to control its lifetime from within `sentry-native`
	NewTransport->Self = NewTransport;

	sentry_transport_t* transport = sentry_transport_new([](sentry_envelope_t* envelope, void* state)
	{
		static_cast<FLinuxSentryTransport*>(state)->Send(envelope);
	});

	sentry_transport_set_state(transport, &NewTransport.Get());

	sentry_transport_set_free_func(transport, [](void* state)
	{
		static_cast<FLinuxSentryTransport*>(state)->Free();
	});
	sentry_transport_set_startup_func(transport, [](const sentry_options_t* options, void* state)->int
	{
		return static_cast<FLinuxSentryTransport*>(state)->Startup(options);
	});
	sentry_transport_set_flush_func(transport, [](uint64_t timeout, void* state)->int
	{
		return static_cast<FLinuxSentryTransport*>(state)->Flush(timeout);
	});
	sentry_transport_set_shutdown_func(transport, [](uint64_t timeout, void* state)->int
	{
		return static_cast<FLinuxSentryTransport*>(state)->Shutdown(timeout);
	});

	return transport;
}

void FLinuxSentryTransport::Send(sentry_envelope_t* envelope)
{
	if(!IsRunning)
	{
		return;
	}

	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	const FSentryDsnUrl DsnUrl(Settings->Dsn);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(FString::Printf(TEXT("https://%s/api/%s/envelope/"), *DsnUrl.GetHost(), *DsnUrl.GetProjectId()));
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/x-sentry-envelope"));
	HttpRequest->SetHeader(TEXT("X-Sentry-Auth"), FString::Printf(TEXT("Sentry sentry_version=7, sentry_client=sentry-unreal, sentry_key=%s"), *DsnUrl.GetKey()));
	HttpRequest->SetContent(FGenericPlatformSentryConverters::SentryEnvelopeToByteArray(envelope));

	HttpRequest->OnProcessRequestComplete().BindThreadSafeSP(this, &FLinuxSentryTransport::OnRequestCompleted);

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

void FLinuxSentryTransport::Free()
{
	Self.Reset();
}

int FLinuxSentryTransport::Startup(const sentry_options_t* options)
{
	IsRunning = true;
	return 0;
}

int FLinuxSentryTransport::Flush(uint64_t timeout)
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

int FLinuxSentryTransport::Shutdown(uint64_t timeout)
{
	IsRunning = false;
	return Flush(timeout);
}

void FLinuxSentryTransport::OnRequestCompleted(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request, TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response, bool bSuccess)
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