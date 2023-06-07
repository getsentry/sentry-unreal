// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "HAL/CriticalSection.h"

#include "Convenience/SentryInclude.h"

#if USE_SENTRY_NATIVE

class FSentryTransport : public TSharedFromThis<FSentryTransport, ESPMode::ThreadSafe>
{
public:
	static sentry_transport_t* Create();

private:
	void Send(sentry_envelope_t* envelope);
	void Free();
	int Startup(const sentry_options_t* options);
	int Flush(uint64_t timeout);
	int Shutdown(uint64_t timeout);

	void OnRequestCompleted(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request, TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response, bool bSuccess);

	TSharedPtr<FSentryTransport, ESPMode::ThreadSafe> Self;

	TArray<TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>> RequestsQueue;

	FCriticalSection CriticalSection;

	bool IsRunning = false;
};

#endif