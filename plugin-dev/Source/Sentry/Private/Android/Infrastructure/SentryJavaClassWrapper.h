// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidJava.h"

class FSentryJavaClassWrapper
{
public:
	FSentryJavaClassWrapper(FName ClassName, const char* CtorSignature, ...);
	FSentryJavaClassWrapper(FName ClassName, jobject JavaClassInstance);
	virtual ~FSentryJavaClassWrapper();

	FSentryJavaClassWrapper(const FSentryJavaClassWrapper& rhs) = delete;
	FSentryJavaClassWrapper& operator = (const FSentryJavaClassWrapper& rhs) = delete;

	FJavaClassMethod GetClassMethod(const char* MethodName, const char* FunctionSignature);

	template<typename ReturnType>
	ReturnType CallMethod(FJavaClassMethod Method, ...) const;

	jobject GetJObject() const;

	void VerifyException() const;

protected:
	jobject Object;
	jclass Class;
};

template<>
void FSentryJavaClassWrapper::CallMethod<void>(FJavaClassMethod Method, ...) const;

template<>
bool FSentryJavaClassWrapper::CallMethod<bool>(FJavaClassMethod Method, ...) const;

template<>
int FSentryJavaClassWrapper::CallMethod<int>(FJavaClassMethod Method, ...) const;

template<>
jobject FSentryJavaClassWrapper::CallMethod<jobject>(FJavaClassMethod Method, ...) const;

template<>
jobjectArray FSentryJavaClassWrapper::CallMethod<jobjectArray>(FJavaClassMethod Method, ...) const;

template<>
int64 FSentryJavaClassWrapper::CallMethod<int64>(FJavaClassMethod Method, ...) const;

template<>
FString FSentryJavaClassWrapper::CallMethod<FString>(FJavaClassMethod Method, ...) const;