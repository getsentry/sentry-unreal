// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySettingsCustomization.h"

TSharedRef<IDetailCustomization> FSentrySettingsCustomization::MakeInstance()
{
	return MakeShareable(new FSentrySettingsCustomization);
}

void FSentrySettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
}
