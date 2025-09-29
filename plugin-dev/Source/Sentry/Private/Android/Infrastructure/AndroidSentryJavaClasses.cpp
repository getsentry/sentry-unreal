// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryJavaClasses.h"

#include "Android/AndroidJavaEnv.h"

// clang-format off

// External Java classes definitions
const FSentryJavaClass SentryJavaClasses::SentryBridgeJava		= FSentryJavaClass { "io/sentry/unreal/SentryBridgeJava", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::Sentry				= FSentryJavaClass { "io/sentry/Sentry", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::Attachment			= FSentryJavaClass { "io/sentry/Attachment", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::Breadcrumb			= FSentryJavaClass { "io/sentry/Breadcrumb", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::SentryEvent			= FSentryJavaClass { "io/sentry/SentryEvent", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::SentryId				= FSentryJavaClass { "io/sentry/protocol/SentryId", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::Scope					= FSentryJavaClass { "io/sentry/IScope", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::ScopeImpl				= FSentryJavaClass { "io/sentry/Scope", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::User					= FSentryJavaClass { "io/sentry/protocol/User", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::Feedback				= FSentryJavaClass { "io/sentry/protocol/Feedback", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::Message				= FSentryJavaClass { "io/sentry/protocol/Message", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::SentryLevel			= FSentryJavaClass { "io/sentry/SentryLevel", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::SentryHint			= FSentryJavaClass { "io/sentry/Hint", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::Transaction			= FSentryJavaClass { "io/sentry/ITransaction", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::Span					= FSentryJavaClass { "io/sentry/ISpan", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::SamplingContext		= FSentryJavaClass { "io/sentry/SamplingContext", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::CustomSamplingContext	= FSentryJavaClass { "io/sentry/CustomSamplingContext", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::TransactionContext	= FSentryJavaClass { "io/sentry/TransactionContext", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::TransactionOptions	= FSentryJavaClass { "io/sentry/TransactionOptions", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::SentryTraceHeader		= FSentryJavaClass { "io/sentry/SentryTraceHeader", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::SentryLogEvent		= FSentryJavaClass { "io/sentry/SentryLogEvent", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::SentryLogLevel		= FSentryJavaClass { "io/sentry/SentryLogLevel", ESentryJavaClassType::External };

// System Java classes definitions
const FSentryJavaClass SentryJavaClasses::ArrayList				= FSentryJavaClass { "java/util/ArrayList", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::HashMap				= FSentryJavaClass { "java/util/HashMap", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::Map					= FSentryJavaClass { "java/util/Map", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::Set					= FSentryJavaClass { "java/util/Set", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::Iterator				= FSentryJavaClass { "java/util/Iterator", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::MapEntry				= FSentryJavaClass { "java/util/Map$Entry", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::List					= FSentryJavaClass { "java/util/List", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::Double				= FSentryJavaClass { "java/lang/Double", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::Integer				= FSentryJavaClass { "java/lang/Integer", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::Float					= FSentryJavaClass { "java/lang/Float", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::Boolean				= FSentryJavaClass { "java/lang/Boolean", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::String				= FSentryJavaClass { "java/lang/String", ESentryJavaClassType::System };

// clang-format on

TMap<FName, jclass> SentryJavaClasses::JavaClassRefsCache;

void SentryJavaClasses::InitJavaClassRefsCache()
{
	// External Java classes definitions
	JavaClassRefsCache.Add(SentryBridgeJava.Name, FindJavaClassRef(SentryBridgeJava));
	JavaClassRefsCache.Add(Sentry.Name, FindJavaClassRef(Sentry));
	JavaClassRefsCache.Add(Attachment.Name, FindJavaClassRef(Attachment));
	JavaClassRefsCache.Add(Breadcrumb.Name, FindJavaClassRef(Breadcrumb));
	JavaClassRefsCache.Add(SentryEvent.Name, FindJavaClassRef(SentryEvent));
	JavaClassRefsCache.Add(SentryId.Name, FindJavaClassRef(SentryId));
	JavaClassRefsCache.Add(Scope.Name, FindJavaClassRef(Scope));
	JavaClassRefsCache.Add(ScopeImpl.Name, FindJavaClassRef(ScopeImpl));
	JavaClassRefsCache.Add(User.Name, FindJavaClassRef(User));
	JavaClassRefsCache.Add(Feedback.Name, FindJavaClassRef(Feedback));
	JavaClassRefsCache.Add(Message.Name, FindJavaClassRef(Message));
	JavaClassRefsCache.Add(SentryLevel.Name, FindJavaClassRef(SentryLevel));
	JavaClassRefsCache.Add(SentryHint.Name, FindJavaClassRef(SentryHint));
	JavaClassRefsCache.Add(Transaction.Name, FindJavaClassRef(Transaction));
	JavaClassRefsCache.Add(Span.Name, FindJavaClassRef(Span));
	JavaClassRefsCache.Add(SamplingContext.Name, FindJavaClassRef(SamplingContext));
	JavaClassRefsCache.Add(CustomSamplingContext.Name, FindJavaClassRef(CustomSamplingContext));
	JavaClassRefsCache.Add(TransactionContext.Name, FindJavaClassRef(TransactionContext));
	JavaClassRefsCache.Add(TransactionOptions.Name, FindJavaClassRef(TransactionOptions));
	JavaClassRefsCache.Add(SentryTraceHeader.Name, FindJavaClassRef(SentryTraceHeader));
	JavaClassRefsCache.Add(SentryLogEvent.Name, FindJavaClassRef(SentryLogEvent));
	JavaClassRefsCache.Add(SentryLogLevel.Name, FindJavaClassRef(SentryLogLevel));

	// System Java classes definitions
	JavaClassRefsCache.Add(ArrayList.Name, FindJavaClassRef(ArrayList));
	JavaClassRefsCache.Add(HashMap.Name, FindJavaClassRef(HashMap));
	JavaClassRefsCache.Add(Map.Name, FindJavaClassRef(Map));
	JavaClassRefsCache.Add(Set.Name, FindJavaClassRef(Set));
	JavaClassRefsCache.Add(Iterator.Name, FindJavaClassRef(Iterator));
	JavaClassRefsCache.Add(MapEntry.Name, FindJavaClassRef(MapEntry));
	JavaClassRefsCache.Add(List.Name, FindJavaClassRef(List));
	JavaClassRefsCache.Add(Double.Name, FindJavaClassRef(Double));
	JavaClassRefsCache.Add(Integer.Name, FindJavaClassRef(Integer));
	JavaClassRefsCache.Add(Float.Name, FindJavaClassRef(Float));
	JavaClassRefsCache.Add(Boolean.Name, FindJavaClassRef(Boolean));
	JavaClassRefsCache.Add(String.Name, FindJavaClassRef(String));
}

void SentryJavaClasses::ClearJavaClassRefsCache()
{
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();

	for (auto& CachedItem : JavaClassRefsCache)
	{
		if (CachedItem.Value)
		{
			JEnv->DeleteGlobalRef(CachedItem.Value);
		}
	}

	JavaClassRefsCache.Empty();
}

jclass SentryJavaClasses::GetCachedJavaClassRef(const FSentryJavaClass& ClassData)
{
	jclass* Class = JavaClassRefsCache.Find(ClassData.Name);

	checkf(Class && *Class, TEXT("Failed to retrieve Java class reference from cache for %s"), *ClassData.Name.ToString());
	return *Class;
}

jclass SentryJavaClasses::FindJavaClassRef(const FSentryJavaClass& ClassData)
{
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();

	ANSICHAR AnsiClassName[NAME_SIZE];
	ClassData.Name.GetPlainANSIString(AnsiClassName);

	jclass Class = nullptr;
	if (ClassData.Type == ESentryJavaClassType::System)
		Class = FJavaWrapper::FindClassGlobalRef(JEnv, AnsiClassName, false);
	else if (ClassData.Type == ESentryJavaClassType::External)
		Class = AndroidJavaEnv::FindJavaClassGlobalRef(AnsiClassName);

	checkf(Class, TEXT("Failed to obtain Java class reference for %s"), *ClassData.Name.ToString());
	return Class;
}
