// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryFeedback.h"

#include "AppleSentryAttachment.h"
#include "AppleSentryId.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

#include "Misc/FileHelper.h"

FAppleSentryFeedback::FAppleSentryFeedback(const FString& message)
	: Message(message)
{
}

FAppleSentryFeedback::~FAppleSentryFeedback()
{
	// Put custom destructor logic here if needed
}

FString FAppleSentryFeedback::GetMessage() const
{
	return Message;
}

void FAppleSentryFeedback::SetName(const FString& name)
{
	Name = name;
}

FString FAppleSentryFeedback::GetName() const
{
	return Name;
}

void FAppleSentryFeedback::SetContactEmail(const FString& email)
{
	Email = email;
}

FString FAppleSentryFeedback::GetContactEmail() const
{
	return Email;
}

void FAppleSentryFeedback::SetAssociatedEvent(const FString& eventId)
{
	EventId = eventId;
}

FString FAppleSentryFeedback::GetAssociatedEvent() const
{
	return EventId;
}

void FAppleSentryFeedback::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	Attachments.Add(attachment);
}

SentryFeedback* FAppleSentryFeedback::CreateSentryFeedback(TSharedPtr<FAppleSentryFeedback> feedback)
{
	SentryId* id = nil;
	if (!feedback->EventId.IsEmpty())
	{
		TSharedPtr<FAppleSentryId> idIOS = MakeShareable(new FAppleSentryId(feedback->EventId));
		id = idIOS->GetNativeObject();
	}

	NSMutableArray<NSData*>* attachments = nil;
	if (feedback->Attachments.Num() > 0)
	{
		attachments = [NSMutableArray arrayWithCapacity:feedback->Attachments.Num()];

		for (const TSharedPtr<ISentryAttachment>& attachment : feedback->Attachments)
		{
			NSData* data = nil;

			if (attachment->GetData().Num() > 0)
			{
				const TArray<uint8>& bytes = attachment->GetData();
				data = [NSData dataWithBytes:bytes.GetData() length:bytes.Num()];
			}
			else if (!attachment->GetPath().IsEmpty())
			{
				TArray<uint8> fileData;
				if (FFileHelper::LoadFileToArray(fileData, *attachment->GetPath()))
				{
					data = [NSData dataWithBytes:fileData.GetData() length:fileData.Num()];
				}
			}

			if (data != nil)
			{
				[attachments addObject:data];
			}
		}
	}

	return [[SENTRY_APPLE_CLASS(SentryFeedback) alloc] initWithMessage:feedback->Message.GetNSString()
																  name:feedback->Name.GetNSString()
																 email:feedback->Email.GetNSString()
																source:SentryFeedbackSourceCustom
													 associatedEventId:id
														   attachments:attachments];
}
