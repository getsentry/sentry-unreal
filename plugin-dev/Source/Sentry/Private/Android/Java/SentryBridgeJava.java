// Copyright (c) 2025 Sentry. All Rights Reserved.

package io.sentry.unreal;

import android.app.Activity;

import androidx.annotation.NonNull;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileInputStream;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import io.sentry.Attachment;
import io.sentry.Breadcrumb;
import io.sentry.Hint;
import io.sentry.IScopes;
import io.sentry.SamplingContext;
import io.sentry.IScope;
import io.sentry.ScopeCallback;
import io.sentry.Sentry;
import io.sentry.SentryEvent;
import io.sentry.SentryLevel;
import io.sentry.SentryOptions;
import io.sentry.android.core.SentryAndroid;
import io.sentry.android.core.SentryAndroidOptions;
import io.sentry.exception.ExceptionMechanismException;
import io.sentry.protocol.Mechanism;
import io.sentry.protocol.SentryException;
import io.sentry.protocol.SentryId;
import io.sentry.SentryLogEvent;

public class SentryBridgeJava {
	public static native void onConfigureScope(long callbackAddr, IScope scope);
	public static native SentryEvent onBeforeSend(long handlerAddr, SentryEvent event, Hint hint);
	public static native Breadcrumb onBeforeBreadcrumb(long handlerAddr, Breadcrumb breadcrumb, Hint hint);
	public static native SentryLogEvent onBeforeLog(long handlerAddr, SentryLogEvent logEvent);
	public static native float onTracesSampler(long samplerAddr, SamplingContext samplingContext);
	public static native String getLogFilePath(boolean isCrash);
	public static native String getScreenshotFilePath();

	public static void init(Activity activity, final String settingsJsonStr) {
		SentryAndroid.init(activity, new Sentry.OptionsConfiguration<SentryAndroidOptions>() {
			@Override
			public void configure(SentryAndroidOptions options) {
				try {
					JSONObject settingJson = new JSONObject(settingsJsonStr);
					options.setDsn(settingJson.getString("dsn"));
					options.setRelease(settingJson.getString("release"));
					options.setEnvironment(settingJson.getString("environment"));
					options.setDist(settingJson.getString("dist"));
					options.setEnableAutoSessionTracking(settingJson.getBoolean("autoSessionTracking"));
					options.setSessionTrackingIntervalMillis(settingJson.getLong("sessionTimeout"));
					options.setAttachStacktrace(settingJson.getBoolean("enableStackTrace"));
					options.setDebug(settingJson.getBoolean("debug"));
					options.setSampleRate(settingJson.getDouble("sampleRate"));
					options.setMaxBreadcrumbs(settingJson.getInt("maxBreadcrumbs"));
					options.setSendDefaultPii(settingJson.getBoolean("sendDefaultPii"));
					JSONArray Includes = settingJson.getJSONArray("inAppInclude");
					for (int i = 0; i < Includes.length(); i++) {
						options.addInAppInclude(Includes.getString(i));
					}
					JSONArray Excludes = settingJson.getJSONArray("inAppExclude");
					for (int i = 0; i < Excludes.length(); i++) {
						options.addInAppExclude(Excludes.getString(i));
					}
					options.setAnrEnabled(settingJson.getBoolean("enableAnrTracking"));
					options.getLogs().setEnabled(settingJson.getBoolean("enableStructuredLogging"));
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
					if(settingJson.has("beforeBreadcrumb")) {
						final long beforeBreadcrumbAddr = settingJson.getLong("beforeBreadcrumb");
						options.setBeforeBreadcrumb(new SentryOptions.BeforeBreadcrumbCallback() {
							@Override
							public Breadcrumb execute(Breadcrumb breadcrumb, Hint hint) {
								return onBeforeBreadcrumb(beforeBreadcrumbAddr, breadcrumb, hint);
							}
						});
					}
                    if (settingJson.has("beforeSendHandler")) {
						options.setBeforeSend(new SentryUnrealBeforeSendCallback(
								settingJson.getBoolean("enableAutoLogAttachment"), settingJson.getBoolean("attachScreenshot"), settingJson.getLong("beforeSendHandler")));
					}
                    else {
						options.setBeforeSend(new SentryUnrealBeforeSendCallback(
								settingJson.getBoolean("enableAutoLogAttachment"), settingJson.getBoolean("attachScreenshot")));
					}

					if (settingJson.has("beforeLogHandler")) {
						options.getLogs().setBeforeSend(new SentryUnrealBeforeLogCallback(settingJson.getLong("beforeLogHandler")));
					}
				} catch (JSONException e) {
					throw new RuntimeException(e);
				}
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

	public static SentryId captureException(final String type, final String value, final Attachment screenshotAttachment) {
		SentryException exception = new SentryException();
		exception.setType(type);
		exception.setValue(value);
		SentryEvent event = new SentryEvent();
		event.setExceptions(Collections.singletonList(exception));

		Hint hint = new Hint();
		if (screenshotAttachment != null) {
			hint.addAttachment(screenshotAttachment);
		}

		SentryId eventId = Sentry.captureEvent(event, hint);
		return eventId;
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
		IScopes scopes = Sentry.getCurrentScopes();
		return scopes.getOptions();
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

	public static void setContext(final SentryEvent event, final String key, final Object values) {
		event.getContexts().put(key, values);
	}

	public static Object getContext(final SentryEvent event, final String key) {
		return event.getContexts().get(key);
	}

	public static void removeContext(final SentryEvent event, final String key) {
		event.getContexts().remove(key);
	}

	public static Object getScopeContext(final IScope scope, final String key) {
		return scope.getContexts().get(key);
	}

	public static void setScopeExtra(final IScope scope, final String key, final Object values) {
		scope.setExtra(key, values.toString());
	}

	public static void addAttachment(final Attachment attachment) {
		Sentry.getGlobalScope().addAttachment(attachment);
	}

	public static void removeAttachment(final Attachment attachment) {
		// Currently, Android SDK doesn't have API allowing to remove individual attachments
	}

	public static void clearAttachments() {
		Sentry.getGlobalScope().clearAttachments();
	}

	public static void addLogFatal(final String message) {
		Sentry.logger().fatal(message);
	}

	public static void addLogError(final String message) {
		Sentry.logger().error(message);
	}

	public static void addLogWarn(final String message) {
		Sentry.logger().warn(message);
	}

	public static void addLogInfo(final String message) {
		Sentry.logger().info(message);
	}

	public static void addLogDebug(final String message) {
		Sentry.logger().debug(message);
	}

	private static class SentryUnrealBeforeSendCallback implements SentryOptions.BeforeSendCallback {
		private final boolean attachLog;
		private final boolean attachScreenshot;
		private final long beforeSendAddr;

		public SentryUnrealBeforeSendCallback(boolean attachLog, boolean attachScreenshot) {
			this.attachLog = attachLog;
			this.attachScreenshot = attachScreenshot;
			this.beforeSendAddr = 0;
		}

		public SentryUnrealBeforeSendCallback(boolean attachLog, boolean attachScreenshot, long beforeSendAddr) {
			this.attachLog = attachLog;
			this.attachScreenshot = attachScreenshot;
			this.beforeSendAddr = beforeSendAddr;
		}

		@Override
		public SentryEvent execute(SentryEvent event, Hint hint) {
			SentryOptions options = getOptions();

			if (attachLog) {
				String logFilePath = getLogFilePath(event.isCrashed());
				if (!logFilePath.isEmpty()) {
					hint.addAttachment(new Attachment(logFilePath, new File(logFilePath).getName(), "text/plain"));
				}
			}

			if (attachScreenshot && event.isCrashed()) {
				String screenshotFilePath = getScreenshotFilePath();
				if (!screenshotFilePath.isEmpty()) {
					try {
						File screenshotFile = new File(screenshotFilePath);
						if (screenshotFile.exists()) {
							byte[] screenshotBytes = readFileToBytes(screenshotFile);
							hint.addAttachment(new Attachment(screenshotBytes, "screenshot.png", "image/png"));
							if (!screenshotFile.delete()) {
								options.getLogger().log(SentryLevel.WARNING, "Failed to delete screenshot: %s", screenshotFilePath);
							}
						}
					} catch (Exception e) {
						options.getLogger().log(SentryLevel.ERROR, "Failed to process screenshot", e);
					}
				}
			}

            if (beforeSendAddr != 0) {
				return onBeforeSend(beforeSendAddr, event, hint);
			}
            return event;
        }
	}

	private static class SentryUnrealBeforeLogCallback implements SentryOptions.Logs.BeforeSendLogCallback {
		private final long beforeLogAddr;

		public SentryUnrealBeforeLogCallback(long beforeLogAddr) {
			this.beforeLogAddr = beforeLogAddr;
		}

		@Override
		public SentryLogEvent execute(SentryLogEvent logEvent) {
			if (beforeLogAddr != 0) {
				return onBeforeLog(beforeLogAddr, logEvent);
			}
			return logEvent;
		}
	}

	private static byte[] readFileToBytes(File file) throws Exception {
		FileInputStream fis = new FileInputStream(file);
		try {
			byte[] buffer = new byte[(int) file.length()];
			int offset = 0;
			int remaining = buffer.length;
			while (remaining > 0) {
				int bytesRead = fis.read(buffer, offset, remaining);
				if (bytesRead < 0) {
					throw new Exception("Unexpected end of file while reading: " + file.getAbsolutePath());
				}
				offset += bytesRead;
				remaining -= bytesRead;
			}
			return buffer;
		} finally {
			fis.close();
		}
	}
}
