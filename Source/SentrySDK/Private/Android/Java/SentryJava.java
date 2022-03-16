// Copyright (c) 2022 Sentry. All Rights Reserved.

package com.sentry.unreal;

import android.app.Activity;

import androidx.annotation.NonNull;

import java.util.concurrent.CountDownLatch;

import io.sentry.Scope;
import io.sentry.ScopeCallback;
import io.sentry.Sentry;
import io.sentry.SentryEvent;
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

	public static String captureMessage(final String message, final SentryLevel level) {
		return Sentry.captureMessage(message, level).toString();
	}

	public static String captureMessageWithScope(final String message, final SentryLevel level, final long callback) throws InterruptedException {
		// TODO Find another solution for returning ID since CountDownLatch blocks game thread
		final CountDownLatch doneSignal = new CountDownLatch(1);
		Sentry.withScope(new ScopeCallback() {
			@Override
			public void run(@NonNull Scope scope) {
				scope.setLevel(level);
				onConfigureScope(callback, scope);
				Sentry.captureMessage(message);
				doneSignal.countDown();
			}
		});
		doneSignal.await();
		return Sentry.getLastEventId().toString();
	}

	public static String captureEvent(final SentryEvent event) {
		return Sentry.captureEvent(event).toString();
	}

	public static String captureEventWithScope(final SentryEvent event, final long callback) throws InterruptedException {
		// TODO Find another solution for returning ID since CountDownLatch blocks game thread
		final CountDownLatch doneSignal = new CountDownLatch(1);
		Sentry.withScope(new ScopeCallback() {
			@Override
			public void run(@NonNull Scope scope) {
				onConfigureScope(callback, scope);
				Sentry.captureEvent(event);
				doneSignal.countDown();
			}
		});
		doneSignal.await();
		return Sentry.getLastEventId().toString();
	}
}