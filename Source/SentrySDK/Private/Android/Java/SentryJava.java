// Copyright (c) 2022 Sentry. All Rights Reserved.

package com.sentry.unreal;

import android.app.Activity;

import io.sentry.Sentry;
import io.sentry.android.core.SentryAndroid;
import io.sentry.android.core.SentryAndroidOptions;

public class SentryJava {
	public static void init(Activity activity, final String dsnUrl) {
		SentryAndroid.init(activity, new Sentry.OptionsConfiguration<SentryAndroidOptions>() {
			@Override
			public void configure(SentryAndroidOptions options) {
				options.setDsn(dsnUrl);
			}
		});
	}
}