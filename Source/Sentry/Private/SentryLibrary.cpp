// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryLibrary.h"
#include "SentryEvent.h"
#include "SentryBreadcrumb.h"
#include "SentryUser.h"
#include "SentryUserFeedback.h"
#include "SentryId.h"
#include "SentryAttachment.h"

USentryEvent* USentryLibrary::CreateSentryEvent(const FString& Message, ESentryLevel Level)
{
	USentryEvent* Event = NewObject<USentryEvent>();

	if(!Message.IsEmpty())
		Event->SetMessage(Message);

	Event->SetLevel(Level);

	return Event;
}

USentryUser* USentryLibrary::CreateSentryUser(const FString& Email, const FString& Id, const FString& Username, const FString& IpAddress, const TMap<FString, FString>& Data)
{
	USentryUser* User = NewObject<USentryUser>();

	if(!Email.IsEmpty())
		User->SetEmail(Email);
	if(!Id.IsEmpty())
		User->SetId(Id);
	if(!Username.IsEmpty())
		User->SetUsername(Username);
	if(!IpAddress.IsEmpty())
		User->SetIpAddress(IpAddress);

	if(Data.Num() > 0)
		User->SetData(Data);

	return User;
}

USentryUserFeedback* USentryLibrary::CreateSentryUserFeedback(USentryId* EventId, const FString& Name, const FString& Email, const FString& Comments)
{
	USentryUserFeedback* UserFeedback = NewObject<USentryUserFeedback>();

	UserFeedback->Initialize(EventId);

	if(!Name.IsEmpty())
		UserFeedback->SetName(Name);
	if(!Email.IsEmpty())
		UserFeedback->SetEmail(Email);
	if(!Comments.IsEmpty())
		UserFeedback->SetComment(Comments);

	return UserFeedback;
}

USentryBreadcrumb* USentryLibrary::CreateSentryBreadcrumb(const FString& Message, const FString& Type, const FString& Category,
	const TMap<FString, FString>& Data, ESentryLevel Level)
{
	USentryBreadcrumb* Breadcrumb = NewObject<USentryBreadcrumb>();

	Breadcrumb->SetMessage(Message);
	Breadcrumb->SetCategory(Category);
	Breadcrumb->SetType(Type);
	Breadcrumb->SetData(Data);
	Breadcrumb->SetLevel(Level);

	return Breadcrumb;
}

USentryAttachment* USentryLibrary::CreateSentryAttachmentWithData(const TArray<uint8>& Data, const FString& Filename, const FString& ContentType)
{
	USentryAttachment* Attachment = NewObject<USentryAttachment>();
	Attachment->InitializeWithData(Data, Filename, ContentType);
	return Attachment;
}

USentryAttachment* USentryLibrary::CreateSentryAttachmentWithPath(const FString& Path, const FString& Filename, const FString& ContentType)
{
	USentryAttachment* Attachment = NewObject<USentryAttachment>();
	Attachment->InitializeWithPath(Path, Filename, ContentType);
	return Attachment;
}

void USentryLibrary::Crash()
{
	char *ptr = 0;
	*ptr += 1;
}

void USentryLibrary::Assert()
{
	char *ptr = nullptr;
	check(ptr != nullptr);
}
