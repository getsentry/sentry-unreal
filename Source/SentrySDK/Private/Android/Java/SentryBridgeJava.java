// Copyright (c) 2022 Sentry. All Rights Reserved.

package com.sentry.unreal;

import android.app.Activity;

import androidx.annotation.NonNull;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.CountDownLatch;

import io.sentry.Breadcrumb;
import io.sentry.Scope;
import io.sentry.ScopeCallback;
import io.sentry.Sentry;
import io.sentry.SentryEvent;
import io.sentry.SentryLevel;
import io.sentry.android.core.SentryAndroid;
import io.sentry.android.core.SentryAndroidOptions;
import io.sentry.protocol.SentryId;

public class SentryBridgeJava {
	public static native void onConfigureScope(long callbackAddr, Scope scope);

	public static void init(Activity activity, final String dsnUrl) {
		SentryAndroid.init(activity, new Sentry.OptionsConfiguration<SentryAndroidOptions>() {
			@Override
			public void configure(SentryAndroidOptions options) {
				options.setDsn(dsnUrl);
			}
		});
	}

	public static void addBreadcrumb(final String message, final String category, final String type, final HashMap<String, String> data, final SentryLevel level) {
		Breadcrumb breadcrumb = new Breadcrumb();
		breadcrumb.setMessage(message);
		breadcrumb.setCategory(category);
		breadcrumb.setType(type);
		breadcrumb.setLevel(level);
		for (Map.Entry<String,String> entry : data.entrySet()) {
			breadcrumb.setData(entry.getKey(), entry.getValue());
		}

		Sentry.addBreadcrumb(breadcrumb);
	}

	public static SentryId captureMessageWithScope(final String message, final SentryLevel level, final long callback) throws InterruptedException {
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
		return Sentry.getLastEventId();
	}

	public static SentryId captureEventWithScope(final SentryEvent event, final long callback) throws InterruptedException {
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
		return Sentry.getLastEventId();
	}
}