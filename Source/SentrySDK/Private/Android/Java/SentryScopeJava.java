// Copyright (c) 2022 Sentry. All Rights Reserved.

package com.sentry.unreal;

import io.sentry.Scope;

public class SentryScopeJava {
	public static void setScopeLevel(final Scope scope, final int level) {
		scope.setLevel(SentryUtilsJava.parseSentryLogLevel(level));
	}
}
