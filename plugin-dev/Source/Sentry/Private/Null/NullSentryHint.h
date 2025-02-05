#pragma once

#include "Interface/SentryHintInterface.h"

class ISentryAttachment;

class FNullSentryHint final : public ISentryHint
{
public:
	virtual ~FNullSentryHint() override = default;

	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) override {}
};
