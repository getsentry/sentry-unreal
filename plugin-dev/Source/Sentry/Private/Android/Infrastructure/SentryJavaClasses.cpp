// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryJavaClasses.h"

// External Java classes definitions
const FSentryJavaClass SentryJavaClasses::SentryBridgeJava	= FSentryJavaClass { "io/sentry/unreal/SentryBridgeJava", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::Sentry			= FSentryJavaClass { "io/sentry/Sentry", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::Attachment		= FSentryJavaClass { "io/sentry/Attachment", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::Breadcrumb		= FSentryJavaClass { "io/sentry/Breadcrumb", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::SentryEvent		= FSentryJavaClass { "io/sentry/SentryEvent", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::SentryId			= FSentryJavaClass { "io/sentry/protocol/SentryId", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::Scope				= FSentryJavaClass { "io/sentry/Scope", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::User				= FSentryJavaClass { "io/sentry/protocol/User", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::UserFeedback		= FSentryJavaClass { "io/sentry/UserFeedback", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::Message			= FSentryJavaClass { "io/sentry/protocol/Message", ESentryJavaClassType::External };
const FSentryJavaClass SentryJavaClasses::SentryLevel		= FSentryJavaClass { "io/sentry/SentryLevel", ESentryJavaClassType::External };

// System Java classes definitions
const FSentryJavaClass SentryJavaClasses::ArrayList			= FSentryJavaClass { "java/util/ArrayList", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::HashMap			= FSentryJavaClass { "java/util/HashMap", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::Map				= FSentryJavaClass { "java/util/Map", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::Set				= FSentryJavaClass { "java/util/Set", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::Iterator			= FSentryJavaClass { "java/util/Iterator", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::MapEntry			= FSentryJavaClass { "java/util/Map$Entry", ESentryJavaClassType::System };
const FSentryJavaClass SentryJavaClasses::List				= FSentryJavaClass { "java/util/List", ESentryJavaClassType::System };