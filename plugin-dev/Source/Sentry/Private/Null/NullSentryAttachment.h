#pragma once

#include "Interface/SentryAttachmentInterface.h"

class FNullSentryAttachment : public ISentryAttachment
{
public:
	virtual ~FNullSentryAttachment() = default;

	virtual TArray<uint8> GetData() const { return {}; }
	virtual FString GetPath() const { return TEXT(""); }
	virtual FString GetFilename() const { return TEXT(""); }
	virtual FString GetContentType() const { return TEXT(""); }
};
