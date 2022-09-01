// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

class FSentrySettingsCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization implementation */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
