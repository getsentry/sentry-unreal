// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "AppleSentryProxy.h"

#if !USE_SENTRY_NATIVE

#include "SentryDefines.h"

@interface FAppleSentryProxyAuthDelegate : NSObject <NSURLSessionTaskDelegate>
@property(nonatomic, copy) NSString* user;
@property(nonatomic, copy) NSString* password;
@end

@implementation FAppleSentryProxyAuthDelegate
- (void)URLSession:(NSURLSession*)session
				   task:(NSURLSessionTask*)task
	didReceiveChallenge:(NSURLAuthenticationChallenge*)challenge
	  completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition, NSURLCredential*))completionHandler
{
	NSURLProtectionSpace* space = challenge.protectionSpace;
	if (!space.isProxy || ![space.authenticationMethod isEqualToString:NSURLAuthenticationMethodHTTPBasic])
	{
		completionHandler(NSURLSessionAuthChallengePerformDefaultHandling, nil);
		return;
	}
	if (self.user == nil || challenge.previousFailureCount > 0)
	{
		completionHandler(NSURLSessionAuthChallengeCancelAuthenticationChallenge, nil);
		return;
	}
	NSString* password = self.password != nil ? self.password : @"";
	completionHandler(NSURLSessionAuthChallengeUseCredential,
		[NSURLCredential credentialWithUser:self.user password:password persistence:NSURLCredentialPersistenceForSession]);
}

- (void)dealloc
{
	[_user release];
	[_password release];
	[super dealloc];
}
@end

NSURLSession* FAppleSentryProxy::CreateUrlSession(const FString& proxyUrl)
{
	NSURLComponents* components = [NSURLComponents componentsWithString:proxyUrl.GetNSString()];
	if (components == nil || components.scheme == nil || components.host.length == 0 || components.port == nil || components.port.integerValue > 65535)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Invalid proxy URL (expected scheme://[user:pass@]host:port). Proxy is ignored"));
		return nil;
	}

	NSString* scheme = components.scheme.lowercaseString;
	if (![scheme isEqualToString:@"http"])
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Unsupported proxy scheme '%s' (only http is supported on Mac/iOS). Proxy is ignored"), *FString(scheme));
		return nil;
	}

	NSURLSessionConfiguration* configuration = [NSURLSessionConfiguration ephemeralSessionConfiguration];
	configuration.connectionProxyDictionary = @{
		@"HTTPEnable" : @YES,
		@"HTTPProxy" : components.host,
		@"HTTPPort" : components.port,
		@"HTTPSEnable" : @YES,
		@"HTTPSProxy" : components.host,
		@"HTTPSPort" : components.port
	};

	FAppleSentryProxyAuthDelegate* delegate = [[[FAppleSentryProxyAuthDelegate alloc] init] autorelease];
	delegate.user = components.user;
	delegate.password = components.password;

	return [NSURLSession sessionWithConfiguration:configuration delegate:delegate delegateQueue:nil];
}

#endif // !USE_SENTRY_NATIVE
