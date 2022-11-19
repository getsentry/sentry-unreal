// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryIdIOS.h"

#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>

SentryIdIOS::SentryIdIOS()
{
	IdIOS = [[SentryId alloc] init];
}

SentryIdIOS::SentryIdIOS(SentryId* id)
{
	IdIOS = id;
}

SentryIdIOS::~SentryIdIOS()
{
	// Put custom destructor logic here if needed
}

SentryId* SentryIdIOS::GetNativeObject()
{
	return IdIOS;
}

FString SentryIdIOS::ToString() const
{
	return FString(IdIOS.sentryIdString);
}
