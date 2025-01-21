#pragma once

#include "Templates/SharedPointer.h"

/**
 * A templatized wrapper to simplify the Unreal <-> Sentry bridging layer.
 */
template<class Interface, class Unreal>
class TSentryImplWrapper
{
public:
	/** Pure virtual destructor to ensure class is abstract. */
	virtual ~TSentryImplWrapper() = 0;

	/** Method to instantiate object with an input native implementation. */
	static Unreal* Create(TSharedPtr<Interface> InObject)
	{
		Unreal* OutObject = NewObject<Unreal>();
		StaticCast<TSentryImplWrapper<Interface, Unreal>*>(OutObject)->NativeImpl = InObject;
		return OutObject;
	}

	/** Retrieves the underlying native implementation. */
	TSharedPtr<Interface> GetNativeObject() const { return NativeImpl; }
protected:
	TSharedPtr<Interface> NativeImpl;
};
