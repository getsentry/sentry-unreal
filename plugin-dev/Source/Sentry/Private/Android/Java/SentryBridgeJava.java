// Copyright (c) 2022 Sentry. All Rights Reserved.

package io.sentry.unreal;

import android.app.Activity;

import androidx.annotation.NonNull;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import io.sentry.Breadcrumb;
import io.sentry.Hint;
import io.sentry.IHub;
import io.sentry.SamplingContext;
import io.sentry.IScope;
import io.sentry.ScopeCallback;
import io.sentry.Sentry;
import io.sentry.SentryEvent;
import io.sentry.SentryLevel;
import io.sentry.SentryOptions;
import io.sentry.android.core.AnrIntegration;
import io.sentry.android.core.SentryAndroid;
import io.sentry.android.core.SentryAndroidOptions;
import io.sentry.exception.ExceptionMechanismException;
import io.sentry.protocol.Mechanism;
import io.sentry.protocol.SentryException;
import io.sentry.protocol.SentryId;
import io.sentry.protocol.SentryStackFrame;
import io.sentry.protocol.SentryStackTrace;

public class SentryBridgeJava {
	public static native void onConfigureScope(long callbackAddr, IScope scope);
	public static native SentryEvent onBeforeSend(long handlerAddr, SentryEvent event, Hint hint);
	public static native float onTracesSampler(long samplerAddr, SamplingContext samplingContext);

	public static void init(Activity activity, final String settingsJsonStr, final long beforeSendHandler) {
		SentryAndroid.init(activity, new Sentry.OptionsConfiguration<SentryAndroidOptions>() {
			@Override
			public void configure(SentryAndroidOptions options) {
				try {
					JSONObject settingJson = new JSONObject(settingsJsonStr);
					options.setDsn(settingJson.getString("dsn"));
					options.setRelease(settingJson.getString("release"));
					options.setEnvironment(settingJson.getString("environment"));
					options.setEnableAutoSessionTracking(settingJson.getBoolean("autoSessionTracking"));
					options.setSessionTrackingIntervalMillis(settingJson.getLong("sessionTimeout"));
					options.setAttachStacktrace(settingJson.getBoolean("enableStackTrace"));
					options.setDebug(settingJson.getBoolean("debug"));
					options.setSampleRate(settingJson.getDouble("sampleRate"));
					options.setMaxBreadcrumbs(settingJson.getInt("maxBreadcrumbs"));
					options.setAttachScreenshot(settingJson.getBoolean("attachScreenshot"));
					options.setSendDefaultPii(settingJson.getBoolean("sendDefaultPii"));
					options.setBeforeSend(new SentryOptions.BeforeSendCallback() {
						@Override
						public SentryEvent execute(SentryEvent event, Hint hint) {
							preProcessEvent(event);
							return onBeforeSend(beforeSendHandler, event, hint);
						}
					});
					JSONArray Includes = settingJson.getJSONArray("inAppInclude");
					for (int i = 0; i < Includes.length(); i++) {
						options.addInAppInclude(Includes.getString(i));
					}
					JSONArray Excludes = settingJson.getJSONArray("inAppExclude");
					for (int i = 0; i < Excludes.length(); i++) {
						options.addInAppExclude(Excludes.getString(i));
					}
					options.setAnrEnabled(settingJson.getBoolean("enableAnrTracking"));
					options.setEnableTracing(settingJson.getBoolean("enableTracing"));
					if(settingJson.has("tracesSampleRate")) {
						options.setTracesSampleRate(settingJson.getDouble("tracesSampleRate"));
					}
					if(settingJson.has("tracesSampler")) {
						final long samplerAddr = settingJson.getLong("tracesSampler");
						options.setTracesSampler(new SentryOptions.TracesSamplerCallback() {
							@Override
							public Double sample(SamplingContext samplingContext) {
							float sampleRate = onTracesSampler(samplerAddr, samplingContext);
							if(sampleRate >= 0.0f) {
								return (double) sampleRate;
							} else {
								return null;
							}
							}
						});
					}
				} catch (JSONException e) {
					throw new RuntimeException(e);
				}
			}
		});
	}

	private static void preProcessEvent(SentryEvent event) {
		if (event.getTags().containsKey("sentry_unreal_exception")) {
			SentryException exception = event.getUnhandledException();
			if (exception != null) {
				exception.setType(event.getTag("sentry_unreal_exception_type"));
				exception.setValue(event.getTag("sentry_unreal_exception_message"));
				SentryStackTrace trace = exception.getStacktrace();
				int numFramesToSkip = Integer.parseInt(event.getTag("sentry_unreal_exception_skip_frames"));
				List<SentryStackFrame> frames = trace.getFrames();
				trace.setFrames(frames.subList(0, frames.size() - numFramesToSkip));
			}
			event.removeTag("sentry_unreal_exception_type");
			event.removeTag("sentry_unreal_exception_message");
			event.removeTag("sentry_unreal_exception_skip_frames");
			event.removeTag("sentry_unreal_exception");
		}
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
			public void run(@NonNull IScope scope) {
			scope.setLevel(level);
			onConfigureScope(callback, scope);
			}
		});
		return messageId;
	}

	public static SentryId captureEventWithScope(final SentryEvent event, final long callback) throws InterruptedException {
		SentryId eventId = Sentry.captureEvent(event, new ScopeCallback() {
			@Override
			public void run(@NonNull IScope scope) {
			onConfigureScope(callback, scope);
			}
		});
		return eventId;
	}

	public static SentryId captureException(final String type, final String value) {
		SentryException exception = new SentryException();
		exception.setType(type);
		exception.setValue(value);
		SentryEvent event = new SentryEvent();
		event.setExceptions(Collections.singletonList(exception));
		SentryId eventId = Sentry.captureEvent(event);
		return eventId;
	}

	public static void configureScope(final long callback) {
		Sentry.configureScope(new ScopeCallback() {
			@Override
			public void run(@NonNull IScope scope) {
				onConfigureScope(callback, scope);
			}
		});
	}

	public static void setContext(final String key, final HashMap<String, String> values) {
		Sentry.configureScope(new ScopeCallback() {
			@Override
			public void run(@NonNull IScope scope) {
				scope.setContexts(key, values);
			}
		});
	}

	public static void setTag(final String key, final String value) {
		Sentry.configureScope(new ScopeCallback() {
			@Override
			public void run(@NonNull IScope scope) {
				scope.setTag(key, value);
			}
		});
	}

	public static void removeTag(final String key) {
		Sentry.configureScope(new ScopeCallback() {
			@Override
			public void run(@NonNull IScope scope) {
				scope.removeTag(key);
			}
		});
	}

	public static void setLevel(final SentryLevel level) {
		Sentry.configureScope(new ScopeCallback() {
			@Override
			public void run(@NonNull IScope scope) {
				scope.setLevel(level);
			}
		});
	}

	public static SentryOptions getOptions() {
		IHub hub = Sentry.getCurrentHub();
		return hub.getOptions();
	}

	public static int isCrashedLastRun() {
		Boolean isCrashed = Sentry.isCrashedLastRun();
		if(isCrashed == null) {
			return -1;
		}
		return isCrashed ? 1 : 0;
	}

	public static boolean isAnrEvent(final SentryEvent event) {
		Throwable throwable = event.getThrowableMechanism();
		if (throwable instanceof ExceptionMechanismException) {
			Mechanism m = ((ExceptionMechanismException) throwable).getExceptionMechanism();
			return m.getType().equals("ANR");
		} else {
			return false;
		}
	}
}