// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#include "SentryEvent.generated.h"

#if PLATFORM_ANDROID
class SentryEventAndroid;
#endif

#if PLATFORM_IOS
class SentryEventIOS;
#endif

UCLASS(BlueprintType)
class SENTRYSDK_API USentryEvent : public UObject
{
	GENERATED_BODY()

public:
	USentryEvent();

	/** Sets message of the event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetMessage(const FString& Message);

	/** Sets the level of the event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetLevel(ESentryLevel Level);

#if PLATFORM_ANDROID
	void InitWithNativeImplAndroid(TSharedPtr<SentryEventAndroid> eventImpl);
	TSharedPtr<SentryEventAndroid> GetNativeImplAndroid();
#endif

#if PLATFORM_IOS
	void InitWithNativeImplIOS(TSharedPtr<SentryEventIOS> eventImpl);
	TSharedPtr<SentryEventIOS> GetNativeImplIOS();
#endif

private:

#if PLATFORM_ANDROID
	TSharedPtr<SentryEventAndroid> _eventNativeImplAndroid;
#endif

#if PLATFORM_IOS
	TSharedPtr<SentryEventIOS> _eventNativeImplIOS;
#endif
};
