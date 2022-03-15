// Copyright (c) 2022 Sentry. All Rights Reserved.

package com.sentry.unreal;

import android.app.Activity;

import androidx.annotation.NonNull;

import java.util.concurrent.CountDownLatch;

import io.sentry.Scope;
import io.sentry.ScopeCallback;
import io.sentry.Sentry;
import io.sentry.SentryLevel;
import io.sentry.android.core.SentryAndroid;
import io.sentry.android.core.SentryAndroidOptions;

public class SentryJava {
	public static native void onConfigureScope(long callbackAddr, Scope scope);

	public static void init(Activity activity, final String dsnUrl) {
		SentryAndroid.init(activity, new Sentry.OptionsConfiguration<SentryAndroidOptions>() {
			@Override
			public void configure(SentryAndroidOptions options) {
				options.setDsn(dsnUrl);
			}
		});
	}

	public static String captureMessage(final String message, final int level) {
		return Sentry.captureMessage(message, parseSentryLogLevel(level)).toString();
	}

	public static String captureMessageWithScope(final String message, final int level, final long callback) throws InterruptedException {
		// TODO Find another solution for returning ID since CountDownLatch blocks game thread
		final CountDownLatch doneSignal = new CountDownLatch(1);
		Sentry.withScope(new ScopeCallback() {
			@Override
			public void run(@NonNull Scope scope) {
				scope.setLevel(parseSentryLogLevel(level));
				onConfigureScope(callback, scope);
				Sentry.captureMessage(message);
				doneSignal.countDown();
			}
		});
		doneSignal.await();
		return Sentry.getLastEventId().toString();
	}

	// TODO Move to another class
	public static void setScopeLevel(final Scope scope, final int level) {
		scope.setLevel(parseSentryLogLevel(level));
	}

	// TODO Move to another class or find a way to use enums with JNI
	private static SentryLevel parseSentryLogLevel(int level) {
		switch (level) {
			case 1:
				return SentryLevel.INFO;
			case 2:
				return SentryLevel.WARNING;
			case 3:
				return SentryLevel.ERROR;
			case 4:
				return SentryLevel.FATAL;
		}

		return SentryLevel.DEBUG;
	}
}