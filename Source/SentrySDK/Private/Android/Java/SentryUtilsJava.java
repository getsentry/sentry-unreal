package com.sentry.unreal;

import io.sentry.SentryLevel;

public class SentryUtilsJava {
	public static SentryLevel parseSentryLogLevel(int level) {
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
