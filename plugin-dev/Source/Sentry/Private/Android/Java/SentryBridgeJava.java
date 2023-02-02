// Copyright (c) 2022 Sentry. All Rights Reserved.

package io.sentry.unreal;

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

	public static void init(Activity activity, final String dsnUrl, final String releaseName, final String environment) {
		SentryAndroid.init(activity, new Sentry.OptionsConfiguration<SentryAndroidOptions>() {
			@Override
			public void configure(SentryAndroidOptions options) {
				options.setDsn(dsnUrl);
				options.setRelease(releaseName);
				options.setEnvironment(environment);
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
		SentryId messageId = Sentry.captureMessage(message, new ScopeCallback() {
			@Override
			public void run(@NonNull Scope scope) {
				scope.setLevel(level);
				onConfigureScope(callback, scope);
			}
		});
		return messageId;
	}

	public static SentryId captureEventWithScope(final SentryEvent event, final long callback) throws InterruptedException {
		SentryId eventId = Sentry.captureEvent(event, new ScopeCallback() {
			@Override
			public void run(@NonNull Scope scope) {
				onConfigureScope(callback, scope);
			}
		});
		return eventId;
	}

	public static void configureScope(final long callback) {
		Sentry.configureScope(new ScopeCallback() {
			@Override
			public void run(@NonNull Scope scope) {
				onConfigureScope(callback, scope);
			}
		});
	}

	public static void setContext(final String key, final HashMap<String, String> values) {
		Sentry.configureScope(new ScopeCallback() {
			@Override
			public void run(@NonNull Scope scope) {
				scope.setContexts(key, values);
			}
		});
	}

	public static void setTag(final String key, final String value) {
		Sentry.configureScope(new ScopeCallback() {
			@Override
			public void run(@NonNull Scope scope) {
				scope.setTag(key, value);
			}
		});
	}

	public static void removeTag(final String key) {
		Sentry.configureScope(new ScopeCallback() {
			@Override
			public void run(@NonNull Scope scope) {
				scope.removeTag(key);
			}
		});
	}

	public static void setLevel(final SentryLevel level) {
		Sentry.configureScope(new ScopeCallback() {
			@Override
			public void run(@NonNull Scope scope) {
				scope.setLevel(level);
			}
		});
	}
}