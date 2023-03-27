// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryJavaObjectWrapper.h"

#include "Android/AndroidJavaEnv.h"

#include "Android/AndroidJNI.h"

FSentryJavaObjectWrapper::FSentryJavaObjectWrapper(FSentryJavaClass ClassData, const char* CtorSignature, ...)
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();

	ANSICHAR AnsiClassName[NAME_SIZE];
	ClassData.Name.GetPlainANSIString(AnsiClassName);

	if(ClassData.Type == ESentryJavaClassType::System)
		Class = FJavaWrapper::FindClassGlobalRef(JEnv, AnsiClassName, false);
	if(ClassData.Type == ESentryJavaClassType::External)
		Class = AndroidJavaEnv::FindJavaClassGlobalRef(AnsiClassName);
	check(Class);

	jmethodID Constructor = JEnv->GetMethodID(Class, "<init>", CtorSignature);
	check(Constructor);

	va_list Params;
	va_start(Params, CtorSignature);
	auto LocalObject = NewScopedJavaObject(JEnv, JEnv->NewObjectV(Class, Constructor, Params));
	va_end(Params);
	VerifyException();
	check(LocalObject);

	Object = JEnv->NewGlobalRef(*LocalObject);
}


FSentryJavaObjectWrapper::FSentryJavaObjectWrapper(FSentryJavaClass ClassData, jobject JavaClassInstance)
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();

	ANSICHAR AnsiClassName[NAME_SIZE];
	ClassData.Name.GetPlainANSIString(AnsiClassName);

	if(ClassData.Type == ESentryJavaClassType::System)
		Class = FJavaWrapper::FindClassGlobalRef(JEnv, AnsiClassName, false);
	if(ClassData.Type == ESentryJavaClassType::External)
		Class = AndroidJavaEnv::FindJavaClassGlobalRef(AnsiClassName);
	check(Class);

	if(!JEnv->IsInstanceOf(JavaClassInstance, Class))
	{
		verify(false && "Java class instance type doesn't match the specified class.");
	}

	Object = JEnv->NewGlobalRef(JavaClassInstance);
}

FSentryJavaObjectWrapper::~FSentryJavaObjectWrapper()
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();
	JEnv->DeleteGlobalRef(Object);
	JEnv->DeleteGlobalRef(Class);
}

FSentryJavaMethod FSentryJavaObjectWrapper::GetMethod(const char* MethodName, const char* FunctionSignature)
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();
	FSentryJavaMethod Method;
	Method.Method = JEnv->GetMethodID(Class, MethodName, FunctionSignature);
	Method.Name = MethodName;
	Method.Signature = FunctionSignature;
	checkf(Method.Method, TEXT("Unable to find Java Method %s with Signature %s"), UTF8_TO_TCHAR(MethodName), UTF8_TO_TCHAR(FunctionSignature));
	return Method;
}

jobject FSentryJavaObjectWrapper::GetJObject() const
{
	check(Object);
	return Object;
}

void FSentryJavaObjectWrapper::VerifyException() const
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();
	if (JEnv->ExceptionCheck())
	{
		JEnv->ExceptionDescribe();
		JEnv->ExceptionClear();
		verify(false && "Java JNI call failed with an exception.");
	}
}

template<>
void FSentryJavaObjectWrapper::CallMethod<void>(FSentryJavaMethod Method, ...) const
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();
	va_list Params;
	va_start(Params, Method);
	JEnv->CallVoidMethodV(Object, Method.Method, Params);
	va_end(Params);
	VerifyException();
}

template<>
bool FSentryJavaObjectWrapper::CallMethod<bool>(FSentryJavaMethod Method, ...) const
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();
	va_list Params;
	va_start(Params, Method);
	bool RetVal = JEnv->CallBooleanMethodV(Object, Method.Method, Params);
	va_end(Params);
	VerifyException();
	return RetVal;
}

template<>
int FSentryJavaObjectWrapper::CallMethod<int>(FSentryJavaMethod Method, ...) const
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();
	va_list Params;
	va_start(Params, Method);
	int RetVal = JEnv->CallIntMethodV(Object, Method.Method, Params);
	va_end(Params);
	VerifyException();
	return RetVal;
}

template<>
jobject FSentryJavaObjectWrapper::CallMethod<jobject>(FSentryJavaMethod Method, ...) const
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();
	va_list Params;
	va_start(Params, Method);
	jobject val = JEnv->CallObjectMethodV(Object, Method.Method, Params);
	va_end(Params);
	VerifyException();
	jobject RetVal = JEnv->NewGlobalRef(val);
	JEnv->DeleteLocalRef(val);
	return RetVal;
}

template<>
jobjectArray FSentryJavaObjectWrapper::CallMethod<jobjectArray>(FSentryJavaMethod Method, ...) const
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();
	va_list Params;
	va_start(Params, Method);
	jobject val = JEnv->CallObjectMethodV(Object, Method.Method, Params);
	va_end(Params);
	VerifyException();
	jobjectArray RetVal = (jobjectArray)JEnv->NewGlobalRef(val);
	JEnv->DeleteLocalRef(val);
	return RetVal;
}

template<>
int64 FSentryJavaObjectWrapper::CallMethod<int64>(FSentryJavaMethod Method, ...) const
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();
	va_list Params;
	va_start(Params, Method);
	int64 RetVal = JEnv->CallLongMethodV(Object, Method.Method, Params);
	va_end(Params);
	VerifyException();
	return RetVal;
}

template<>
FString FSentryJavaObjectWrapper::CallMethod<FString>(FSentryJavaMethod Method, ...) const
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();
	va_list Params;
	va_start(Params, Method);
	jstring RetVal = static_cast<jstring>(JEnv->CallObjectMethodV(Object, Method.Method, Params));
	va_end(Params);
	VerifyException();
	auto Result = FJavaHelper::FStringFromLocalRef(JEnv, RetVal);
	return Result;
}