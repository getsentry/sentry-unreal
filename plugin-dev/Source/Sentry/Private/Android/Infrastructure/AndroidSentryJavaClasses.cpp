// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryJavaClasses.h"

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
const FSentryJavaClass SentryJavaClasses::UserFeedback			= FSentryJavaClass { "io/sentry/UserFeedback", ESentryJavaClassType::External };
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