// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

// clang-format off

#ifdef SENTRY_PLATFORM_NULL
	#define SENTRY_COMPILED_PLATFORM_HEADER(Suffix) \
		PREPROCESSOR_TO_STRING(PREPROCESSOR_JOIN(Null/Null, Suffix))
#else
	#define SENTRY_COMPILED_PLATFORM_HEADER(Suffix) \
		COMPILED_PLATFORM_HEADER(Suffix)
#endif

// clang-format on