// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryJavaClassWrapper.h"

#include "Android/AndroidJavaEnv.h"

FSentryJavaClassWrapper::FSentryJavaClassWrapper(FName ClassName, const char* CtorSignature, ...)
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();

	ANSICHAR AnsiClassName[NAME_SIZE];
	ClassName.GetPlainANSIString(AnsiClassName);

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


FSentryJavaClassWrapper::FSentryJavaClassWrapper(FName ClassName, jobject JavaClassInstance)
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();

	ANSICHAR AnsiClassName[NAME_SIZE];
	ClassName.GetPlainANSIString(AnsiClassName);

	Class = AndroidJavaEnv::FindJavaClassGlobalRef(AnsiClassName);
	check(Class);

	if(!JEnv->IsInstanceOf(JavaClassInstance, Class))
	{
		verify(false && "Java class instance type doesn't match the specified class.");
	}

	Object = JEnv->NewGlobalRef(JavaClassInstance);
}

FSentryJavaClassWrapper::~FSentryJavaClassWrapper()
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();
	JEnv->DeleteGlobalRef(Object);
	JEnv->DeleteGlobalRef(Class);
}

FSentryJavaClassMethod FSentryJavaClassWrapper::GetClassMethod(const char* MethodName, const char* FunctionSignature)
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();
	FSentryJavaClassMethod Method;
	Method.Method = JEnv->GetMethodID(Class, MethodName, FunctionSignature);
	Method.Name = MethodName;
	Method.Signature = FunctionSignature;
	checkf(Method.Method, TEXT("Unable to find Java Method %s with Signature %s"), UTF8_TO_TCHAR(MethodName), UTF8_TO_TCHAR(FunctionSignature));
	return Method;
}

jobject FSentryJavaClassWrapper::GetJObject() const
{
	check(Object);
	return Object;
}

void FSentryJavaClassWrapper::VerifyException() const
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
void FSentryJavaClassWrapper::CallMethod<void>(FSentryJavaClassMethod Method, ...) const
{
	JNIEnv*	JEnv = AndroidJavaEnv::GetJavaEnv();
	va_list Params;
	va_start(Params, Method);
	JEnv->CallVoidMethodV(Object, Method.Method, Params);
	va_end(Params);
	VerifyException();
}

template<>
bool FSentryJavaClassWrapper::CallMethod<bool>(FSentryJavaClassMethod Method, ...) const
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
int FSentryJavaClassWrapper::CallMethod<int>(FSentryJavaClassMethod Method, ...) const
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
jobject FSentryJavaClassWrapper::CallMethod<jobject>(FSentryJavaClassMethod Method, ...) const
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
jobjectArray FSentryJavaClassWrapper::CallMethod<jobjectArray>(FSentryJavaClassMethod Method, ...) const
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
int64 FSentryJavaClassWrapper::CallMethod<int64>(FSentryJavaClassMethod Method, ...) const
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
FString FSentryJavaClassWrapper::CallMethod<FString>(FSentryJavaClassMethod Method, ...) const
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