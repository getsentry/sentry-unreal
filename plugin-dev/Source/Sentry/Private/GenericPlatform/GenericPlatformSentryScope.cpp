// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "GenericPlatformSentryScope.h"
#include "GenericPlatformSentryBreadcrumb.h"
#include "GenericPlatformSentryEvent.h"

#include "Interface/SentryAttachmentInterface.h"

#include "SentryModule.h"
#include "SentrySettings.h"

#include "Infrastructure/GenericPlatformSentryConverters.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentryScope::FGenericPlatformSentryScope()
	: Level(ESentryLevel::Debug)
{
}

FGenericPlatformSentryScope::FGenericPlatformSentryScope(const FGenericPlatformSentryScope& Scope)
{
	Dist = Scope.Dist;
	Environment = Scope.Environment;
	Fingerprint = Scope.Fingerprint;
	Tags = Scope.Tags;
	Extra = Scope.Extra;
	Contexts = Scope.Contexts;
	Breadcrumbs = TRingBuffer<TSharedPtr<FGenericPlatformSentryBreadcrumb>>(Scope.Breadcrumbs);
	Level = Scope.Level;
}

FGenericPlatformSentryScope::~FGenericPlatformSentryScope()
{
}

void FGenericPlatformSentryScope::AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb)
{
	FScopeLock Lock(&CriticalSection);

	if(Breadcrumbs.Num() >= FSentryModule::Get().GetSettings()->MaxBreadcrumbs)
	{
		Breadcrumbs.PopFront();
	}

	Breadcrumbs.Add(StaticCastSharedPtr<FGenericPlatformSentryBreadcrumb>(breadcrumb));
}

void FGenericPlatformSentryScope::ClearBreadcrumbs()
{
	FScopeLock Lock(&CriticalSection);

	Breadcrumbs.Empty();
}

void FGenericPlatformSentryScope::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	// Not available for generic platform
}

void FGenericPlatformSentryScope::ClearAttachments()
{
	// Not available for generic platform
}

void FGenericPlatformSentryScope::SetTagValue(const FString& key, const FString& value)
{
	Tags.Add(key, value);
}

FString FGenericPlatformSentryScope::GetTagValue(const FString& key) const
{
	if(!Tags.Contains(key))
		return FString();

	return Tags[key];
}

void FGenericPlatformSentryScope::RemoveTag(const FString& key)
{
	Tags.Remove(key);
}

void FGenericPlatformSentryScope::SetTags(const TMap<FString, FString>& tags)
{
	Tags.Append(tags);
}

TMap<FString, FString> FGenericPlatformSentryScope::GetTags() const
{
	return Tags;
}

void FGenericPlatformSentryScope::SetDist(const FString& dist)
{
	Dist = dist;
}

FString FGenericPlatformSentryScope::GetDist() const
{
	return Dist;
}

void FGenericPlatformSentryScope::SetEnvironment(const FString& environment)
{
	Environment = environment;
}

FString FGenericPlatformSentryScope::GetEnvironment() const
{
	return Environment;
}

void FGenericPlatformSentryScope::SetFingerprint(const TArray<FString>& fingerprint)
{
	Fingerprint = fingerprint;
}

TArray<FString> FGenericPlatformSentryScope::GetFingerprint() const
{
	return Fingerprint;
}

void FGenericPlatformSentryScope::SetLevel(ESentryLevel level)
{
	Level = level;
}

ESentryLevel FGenericPlatformSentryScope::GetLevel() const
{

	return Level;
}

void FGenericPlatformSentryScope::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	Contexts.Add(key, values);
}

void FGenericPlatformSentryScope::RemoveContext(const FString& key)
{
	if(!Contexts.Contains(key))
		return;

	Contexts.Remove(key);
}

void FGenericPlatformSentryScope::SetExtraValue(const FString& key, const FString& value)
{
	Extra.Add(key, value);
}

FString FGenericPlatformSentryScope::GetExtraValue(const FString& key) const
{
	if(!Extra.Contains(key))
		return FString();

	return Extra[key];
}

void FGenericPlatformSentryScope::RemoveExtra(const FString& key)
{
	if(!Extra.Contains(key))
		return;

	Extra.Remove(key);
}

void FGenericPlatformSentryScope::SetExtras(const TMap<FString, FString>& extras)
{
	Extra.Append(extras);
}

TMap<FString, FString> FGenericPlatformSentryScope::GetExtras() const
{
	return Extra;
}

void FGenericPlatformSentryScope::Clear()
{
	Dist = FString();
	Environment = FString();
	Fingerprint.Empty();
	Tags.Empty();
	Extra.Empty();
	Contexts.Empty();
	Breadcrumbs.Empty();
	Level = ESentryLevel::Debug;
}

void FGenericPlatformSentryScope::Apply(TSharedPtr<FGenericPlatformSentryEvent> event)
{
	sentry_value_t nativeEvent = event->GetNativeObject();

	sentry_value_t eventLevel = sentry_value_get_by_key(nativeEvent, "level");

	FString scopeLevelStr = FGenericPlatformSentryConverters::SentryLevelToString(Level).ToLower();
	if (!scopeLevelStr.IsEmpty() && sentry_value_is_null(eventLevel))
	{
		sentry_value_set_by_key(nativeEvent, "level", sentry_value_new_string(TCHAR_TO_ANSI(*scopeLevelStr)));
	}

	if(!Dist.IsEmpty())
	{
		sentry_value_set_by_key(nativeEvent, "dist", sentry_value_new_string(TCHAR_TO_ANSI(*Dist)));
	}

	if(!Environment.IsEmpty())
	{
		sentry_value_set_by_key(nativeEvent, "environment", sentry_value_new_string(TCHAR_TO_ANSI(*Environment)));
	}

	if(Fingerprint.Num() > 0)
	{
		sentry_value_set_by_key(nativeEvent, "fingerprint", FGenericPlatformSentryConverters::StringArrayToNative(Fingerprint));
	}

	if(Tags.Num() > 0)
	{
		sentry_value_t tagsExtra = sentry_value_get_by_key(nativeEvent, "tags");
		if(sentry_value_is_null(tagsExtra))
		{
			sentry_value_set_by_key(nativeEvent, "tags", FGenericPlatformSentryConverters::StringMapToNative(Tags));
		}
		else
		{
			for (const auto& TagItem : Tags)
			{
				sentry_value_set_by_key(tagsExtra, TCHAR_TO_ANSI(*TagItem.Key), sentry_value_new_string(TCHAR_TO_ANSI(*TagItem.Value)));
			}
		}
	}

	if(Extra.Num() > 0)
	{
		sentry_value_t eventExtra = sentry_value_get_by_key(nativeEvent, "extra");
		if(sentry_value_is_null(eventExtra))
		{
			sentry_value_set_by_key(nativeEvent, "extra", FGenericPlatformSentryConverters::StringMapToNative(Extra));
		}
		else
		{
			for (const auto& ExtraItem : Extra)
			{
				sentry_value_set_by_key(eventExtra, TCHAR_TO_ANSI(*ExtraItem.Key), sentry_value_new_string(TCHAR_TO_ANSI(*ExtraItem.Value)));
			}
		}
		
	}

	if(Contexts.Num() > 0)
	{
		sentry_value_t eventContexts = sentry_value_get_by_key(nativeEvent, "contexts");
		if(sentry_value_is_null(eventContexts))
		{
			eventContexts = sentry_value_new_object();

			for (const auto& ContextsItem : Contexts)
			{
				sentry_value_set_by_key(eventContexts, TCHAR_TO_ANSI(*ContextsItem.Key), FGenericPlatformSentryConverters::StringMapToNative(ContextsItem.Value));
			}

			sentry_value_set_by_key(nativeEvent, "contexts", eventContexts);
		}
		else
		{
			for (const auto& ContextsItem : Contexts)
			{
				sentry_value_set_by_key(eventContexts, TCHAR_TO_ANSI(*ContextsItem.Key), FGenericPlatformSentryConverters::StringMapToNative(ContextsItem.Value));
			}
		}
	}

	if(!Breadcrumbs.IsEmpty())
	{
		sentry_value_t eventBreadcrumbs = sentry_value_get_by_key(nativeEvent, "breadcrumbs");
		if(sentry_value_is_null(eventBreadcrumbs))
		{
			eventBreadcrumbs = sentry_value_new_list();

			for (const auto& Breadcrumb : Breadcrumbs)
			{
				sentry_value_t nativeBreadcrumb = Breadcrumb->GetNativeObject();
				sentry_value_incref(nativeBreadcrumb);
				sentry_value_append(eventBreadcrumbs, nativeBreadcrumb);
			}

			sentry_value_set_by_key(nativeEvent, "breadcrumbs", eventBreadcrumbs);
		}
		else
		{
			for (const auto& Breadcrumb : Breadcrumbs)
			{
				sentry_value_t nativeBreadcrumb = Breadcrumb->GetNativeObject();
				sentry_value_incref(nativeBreadcrumb);
				sentry_value_append(eventBreadcrumbs, nativeBreadcrumb);
			}
		}
	}
}

#endif
