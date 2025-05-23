// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryEventInterface.h"

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class FAndroidSentryEvent : public ISentryEvent, public FSentryJavaObjectWrapper
{
public:
	FAndroidSentryEvent();
	FAndroidSentryEvent(jobject event);

	void SetupClassMethods();

	virtual TSharedPtr<ISentryId> GetId() const override;
	virtual void SetMessage(const FString& message) override;
	virtual FString GetMessage() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;
	virtual void SetFingerprint(const TArray<FString>& fingerprint) override;
	virtual TArray<FString> GetFingerprint() override;
	virtual bool IsCrash() const override;
	virtual bool IsAnr() const override;

private:
	FSentryJavaMethod GetIdMethod;
	FSentryJavaMethod SetMessageMethod;
	FSentryJavaMethod GetMessageMethod;
	FSentryJavaMethod SetLevelMethod;
	FSentryJavaMethod GetLevelMethod;
	FSentryJavaMethod SetFingerprintMethod;
	FSentryJavaMethod GetFingerprintMethod;
	FSentryJavaMethod IsCrashMethod;
};

typedef FAndroidSentryEvent FPlatformSentryEvent;