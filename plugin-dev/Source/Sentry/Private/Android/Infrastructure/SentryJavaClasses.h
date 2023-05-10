// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypesAndroid.h"

struct SentryJavaClasses
{
	// External Java classes
	const static FSentryJavaClass SentryBridgeJava;
	const static FSentryJavaClass Sentry;
	const static FSentryJavaClass Attachment;
	const static FSentryJavaClass Breadcrumb;
	const static FSentryJavaClass SentryEvent;
	const static FSentryJavaClass SentryId;
	const static FSentryJavaClass Scope;
	const static FSentryJavaClass User;
	const static FSentryJavaClass UserFeedback;
	const static FSentryJavaClass Message;
	const static FSentryJavaClass SentryLevel;

	// System Java classes
	const static FSentryJavaClass ArrayList;
	const static FSentryJavaClass HashMap;
	const static FSentryJavaClass Map;
	const static FSentryJavaClass Set;
	const static FSentryJavaClass Iterator;
	const static FSentryJavaClass MapEntry;
	const static FSentryJavaClass List;
};