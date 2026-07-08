// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "AppleSentryReplayEnvelope.h"

#if !USE_SENTRY_NATIVE

#include "SentryDefines.h"

#include "SessionReplay/SentryReplayInfo.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{

void AppendMsgPackString(NSMutableData* data, const char* str)
{
	const uint8 header[] = { 0xD9, static_cast<uint8>(strlen(str)) };
	[data appendBytes:header length:sizeof(header)];
	[data appendBytes:str length:strlen(str)];
}

void AppendMsgPackBinary(NSMutableData* data, NSData* payload)
{
	const uint32 length = static_cast<uint32>(payload.length);
	const uint8 header[] = { 0xC6,
		static_cast<uint8>((length >> 24) & 0xFF), static_cast<uint8>((length >> 16) & 0xFF),
		static_cast<uint8>((length >> 8) & 0xFF), static_cast<uint8>(length & 0xFF) };
	[data appendBytes:header length:sizeof(header)];
	[data appendData:payload];
}

id SanitizeForJson(id value)
{
	if ([value isKindOfClass:[NSDictionary class]])
	{
		NSDictionary* dict = (NSDictionary*)value;
		NSMutableDictionary* result = [NSMutableDictionary dictionaryWithCapacity:dict.count];
		for (id key in dict)
		{
			result[[key description]] = SanitizeForJson(dict[key]);
		}
		return result;
	}
	if ([value isKindOfClass:[NSArray class]])
	{
		NSArray* array = (NSArray*)value;
		NSMutableArray* result = [NSMutableArray arrayWithCapacity:array.count];
		for (id item in array)
		{
			[result addObject:SanitizeForJson(item)];
		}
		return result;
	}
	if ([value isKindOfClass:[NSString class]] || [value isKindOfClass:[NSNumber class]] || [value isKindOfClass:[NSNull class]])
	{
		return value;
	}
	if ([value isKindOfClass:[NSDate class]])
	{
		return @([(NSDate*)value timeIntervalSince1970]);
	}
	return [value description];
}

NSDictionary* SerializeUser(SentryObjCUser* user)
{
	NSMutableDictionary* dict = [NSMutableDictionary dictionary];
	if (user.userId)
		dict[@"id"] = user.userId;
	if (user.email)
		dict[@"email"] = user.email;
	if (user.username)
		dict[@"username"] = user.username;
	if (user.ipAddress)
		dict[@"ip_address"] = user.ipAddress;
	if (user.name)
		dict[@"name"] = user.name;
	if (user.geo)
	{
		NSMutableDictionary* geo = [NSMutableDictionary dictionary];
		if (user.geo.city)
			geo[@"city"] = user.geo.city;
		if (user.geo.countryCode)
			geo[@"country_code"] = user.geo.countryCode;
		if (user.geo.region)
			geo[@"region"] = user.geo.region;
		dict[@"geo"] = geo;
	}
	if (user.data)
		dict[@"data"] = user.data;
	return dict;
}

NSDictionary* BuildReplayEvent(SentryObjCEvent* event, const FSentryReplayInfo& info, double startSec, double endSec)
{
	NSMutableDictionary* dict = [NSMutableDictionary dictionary];
	dict[@"type"] = @"replay_event";
	dict[@"replay_type"] = info.ReplayType.IsEmpty() ? @"buffer" : info.ReplayType.GetNSString();
	dict[@"segment_id"] = @(info.SegmentId);
	dict[@"replay_id"] = info.ReplayId.GetNSString();
	dict[@"event_id"] = info.ReplayId.GetNSString();
	dict[@"platform"] = @"cocoa";
	dict[@"timestamp"] = @(endSec);
	dict[@"replay_start_timestamp"] = @(startSec);
	dict[@"urls"] = @[];

	if (event != nil)
	{
		if (event.tags)
			dict[@"tags"] = event.tags;
		if (event.context)
			dict[@"contexts"] = event.context;
		if (event.releaseName)
			dict[@"release"] = event.releaseName;
		if (event.environment)
			dict[@"environment"] = event.environment;
		if (event.dist)
			dict[@"dist"] = event.dist;
		if (event.user)
			dict[@"user"] = SerializeUser(event.user);
		if (event.sdk)
			dict[@"sdk"] = event.sdk;

		id traceId = event.context[@"trace"][@"trace_id"];
		dict[@"trace_ids"] = [traceId isKindOfClass:[NSString class]] ? @[ traceId ] : @[];
	}

	return SanitizeForJson(dict);
}

NSData* BuildReplayRecording(const FSentryReplayInfo& info, double startSec, uint64 videoSizeBytes)
{
	const double timestampMs = startSec * 1000.0;

	NSDictionary* metaEvent = @{
		@"type" : @4,
		@"timestamp" : @(timestampMs),
		@"data" : @{
			@"href" : @"",
			@"width" : @(info.Width),
			@"height" : @(info.Height)
		}
	};

	NSDictionary* videoEvent = @{
		@"type" : @5,
		@"timestamp" : @(timestampMs),
		@"data" : @{
			@"tag" : @"video",
			@"payload" : @{
				@"segmentId" : @(info.SegmentId),
				@"size" : @(static_cast<double>(videoSizeBytes)),
				@"duration" : @(static_cast<double>(info.DurationMs)),
				@"encoding" : @"h264",
				@"container" : @"mp4",
				@"height" : @(info.Height),
				@"width" : @(info.Width),
				@"left" : @0,
				@"top" : @0,
				@"frameCount" : @(info.FrameCount),
				@"frameRate" : @(info.FrameRate),
				@"frameRateType" : @"variable"
			}
		}
	};

	NSError* error = nil;
	NSData* eventsJson = [NSJSONSerialization dataWithJSONObject:@[ metaEvent, videoEvent ] options:0 error:&error];
	if (eventsJson == nil)
	{
		return nil;
	}

	NSMutableData* recording = [NSMutableData data];
	NSString* header = [NSString stringWithFormat:@"{\"segment_id\":%d}\n", info.SegmentId];
	[recording appendData:[header dataUsingEncoding:NSUTF8StringEncoding]];
	[recording appendData:eventsJson];
	return recording;
}

} // namespace

bool FAppleSentryReplayEnvelope::CaptureForCrashEvent(SentryObjCEvent* event, const FString& videoPath, const FString& sidecarPath)
{
	FString sidecarJson;
	if (!FFileHelper::LoadFileToString(sidecarJson, *sidecarPath))
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: failed to read metadata sidecar at %s"), *sidecarPath);
		return false;
	}

	FSentryReplayInfo info;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(sidecarJson, &info) || info.ReplayId.IsEmpty())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: failed to parse metadata sidecar at %s"), *sidecarPath);
		return false;
	}

	if (FPaths::GetBaseFilename(videoPath) != FString::Printf(TEXT("replay-%s"), *info.ReplayId))
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: metadata sidecar %s doesn't match video file %s"), *sidecarPath, *videoPath);
		return false;
	}

	NSData* video = [NSData dataWithContentsOfFile:videoPath.GetNSString()];
	if (video == nil || video.length == 0)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: failed to read video file at %s"), *videoPath);
		return false;
	}

	// The replay window ends at the crash time so the corresponding error appears
	// on the replay timeline; the sidecar's own end timestamp is a fallback only
	const double endSec = (event != nil && event.timestamp != nil)
							  ? [event.timestamp timeIntervalSince1970]
							  : info.EndTimestampSec;
	const double startSec = endSec - static_cast<double>(info.DurationMs) / 1000.0;

	NSDictionary* replayEvent = BuildReplayEvent(event, info, startSec, endSec);
	if (![NSJSONSerialization isValidJSONObject:replayEvent])
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: replay event is not serializable to JSON"));
		return false;
	}

	NSError* error = nil;
	NSData* replayEventJson = [NSJSONSerialization dataWithJSONObject:replayEvent options:0 error:&error];
	if (replayEventJson == nil)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: failed to serialize replay event"));
		return false;
	}

	NSData* replayRecording = BuildReplayRecording(info, startSec, video.length);
	if (replayRecording == nil)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: failed to serialize replay recording"));
		return false;
	}

	NSMutableData* payload = [NSMutableData dataWithCapacity:replayEventJson.length + replayRecording.length + video.length + 64];
	const uint8 mapHeader = 0x83;
	[payload appendBytes:&mapHeader length:sizeof(mapHeader)];
	AppendMsgPackString(payload, "replay_event");
	AppendMsgPackBinary(payload, replayEventJson);
	AppendMsgPackString(payload, "replay_recording");
	AppendMsgPackBinary(payload, replayRecording);
	AppendMsgPackString(payload, "replay_video");
	AppendMsgPackBinary(payload, video);

	SentryObjCEnvelopeItem* envelopeItem = [[SENTRY_APPLE_CLASS(SentryObjCEnvelopeItem) alloc] initWithType:@"replay_video"
																									   data:payload
																								addPlatform:NO];

	SentryObjCId* replayId = [[SENTRY_APPLE_CLASS(SentryObjCId) alloc] initWithUUIDString:info.ReplayId.GetNSString()];

	SentryObjCEnvelopeHeader* envelopeHeader = [[SENTRY_APPLE_CLASS(SentryObjCEnvelopeHeader) alloc] initWithId:replayId traceContext:nil];

	SentryObjCEnvelope* envelope = [[SENTRY_APPLE_CLASS(SentryObjCEnvelope) alloc] initWithHeader:envelopeHeader singleItem:envelopeItem];
	if (envelope == nil)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: failed to create replay envelope"));
		return false;
	}

	[[[SENTRY_APPLE_CLASS(SentryObjCSDK) internal] envelope] capture:envelope];
	return true;
}

#endif // !USE_SENTRY_NATIVE
