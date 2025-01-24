#pragma once

#include "Interface/SentryAttachmentInterface.h"

class FNullSentryAttachment : public ISentryAttachment
{
public:
	virtual ~FNullSentryAttachment() override = default;

	virtual TArray<uint8> GetData() const override { return {}; }
	virtual FString GetPath() const override { return TEXT(""); }
	virtual FString GetFilename() const override { return TEXT(""); }
	virtual FString GetContentType() const override { return TEXT(""); }
};
