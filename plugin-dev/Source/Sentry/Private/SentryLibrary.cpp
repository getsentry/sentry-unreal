// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryLibrary.h"
#include "SentryAttachment.h"
#include "SentryBreadcrumb.h"
#include "SentryEvent.h"
#include "SentryFeedback.h"
#include "SentryTransactionContext.h"
#include "SentryUser.h"

#include "HAL/PlatformSentryAttachment.h"
#include "HAL/PlatformSentryBreadcrumb.h"
#include "HAL/PlatformSentryEvent.h"
#include "HAL/PlatformSentryFeedback.h"
#include "HAL/PlatformSentryTransactionContext.h"
#include "HAL/PlatformSentryUser.h"

USentryEvent* USentryLibrary::CreateSentryEvent(const FString& Message, ESentryLevel Level)
{
	USentryEvent* Event = USentryEvent::Create(CreateSharedSentryEvent());

	if (!Message.IsEmpty())
		Event->SetMessage(Message);

	Event->SetLevel(Level);

	return Event;
}

USentryUser* USentryLibrary::CreateSentryUser(const FString& Email, const FString& Id, const FString& Username, const FString& IpAddress, const TMap<FString, FString>& Data)
{
	USentryUser* User = USentryUser::Create(MakeShareable(new FPlatformSentryUser));

	if (!Email.IsEmpty())
		User->SetEmail(Email);
	if (!Id.IsEmpty())
		User->SetId(Id);
	if (!Username.IsEmpty())
		User->SetUsername(Username);
	if (!IpAddress.IsEmpty())
		User->SetIpAddress(IpAddress);

	if (Data.Num() > 0)
		User->SetData(Data);

	return User;
}

USentryFeedback* USentryLibrary::CreateSentryFeedback(const FString& Message, const FString& Name, const FString& Email, const FString& EventId)
{
	USentryFeedback* Feedback = USentryFeedback::Create(MakeShareable(new FPlatformSentryFeedback(Message)));

	if (!Name.IsEmpty())
		Feedback->SetName(Name);
	if (!Email.IsEmpty())
		Feedback->SetContactEmail(Email);
	if (!EventId.IsEmpty())
		Feedback->SetAssociatedEvent(EventId);

	return Feedback;
}

USentryBreadcrumb* USentryLibrary::CreateSentryBreadcrumb(const FString& Message, const FString& Type, const FString& Category,
	const TMap<FString, FSentryVariant>& Data, ESentryLevel Level)
{
	USentryBreadcrumb* Breadcrumb = USentryBreadcrumb::Create(CreateSharedSentryBreadcrumb());

	Breadcrumb->SetMessage(Message);
	Breadcrumb->SetCategory(Category);
	Breadcrumb->SetType(Type);
	Breadcrumb->SetData(Data);
	Breadcrumb->SetLevel(Level);

	return Breadcrumb;
}

USentryAttachment* USentryLibrary::CreateSentryAttachmentWithData(const TArray<uint8>& Data, const FString& Filename, const FString& ContentType)
{
	return USentryAttachment::Create(CreateSharedSentryAttachment(Data, Filename, ContentType));
}

USentryAttachment* USentryLibrary::CreateSentryAttachmentWithPath(const FString& Path, const FString& Filename, const FString& ContentType)
{
	return USentryAttachment::Create(CreateSharedSentryAttachment(Path, Filename, ContentType));
}

USentryTransactionContext* USentryLibrary::CreateSentryTransactionContext(const FString& Name, const FString& Operation)
{
	return USentryTransactionContext::Create(CreateSharedSentryTransactionContext(Name, Operation));
}
