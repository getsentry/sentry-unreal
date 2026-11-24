// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryAttachment.h"
#include "SentryLibrary.h"
#include "SentryTests.h"

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryAttachmentSpec, "Sentry.SentryAttachment", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	USentryAttachment* SentryAttachment;
END_DEFINE_SPEC(SentryAttachmentSpec)

void SentryAttachmentSpec::Define()
{
	BeforeEach([this]()
	{
		SentryAttachment = NewObject<USentryAttachment>();
	});

	Describe("Data attachment", [this]()
	{
		It("should persist its values", [this]()
		{
			const TArray<uint8> TestData = { 0x01, 0x00, 0x02, 0xFF };
			const FString TestFilename = TEXT("data.bin");
			const FString TestContentType = TEXT("application/x-test");

			SentryAttachment->InitializeWithData(TestData, TestFilename, TestContentType);

			TestTrue("Data", SentryAttachment->GetData() == TestData);
			TestEqual("Filename", SentryAttachment->GetFilename(), TestFilename);
			TestEqual("Content type", SentryAttachment->GetContentType(), TestContentType);
			TestTrue("Path", SentryAttachment->GetPath().IsEmpty());
		});

		It("should use default content type if not specified", [this]()
		{
			SentryAttachment->InitializeWithData({ 0x01 }, TEXT("data.bin"));

			TestEqual("Content type", SentryAttachment->GetContentType(), TEXT("application/octet-stream"));
		});

		It("should strip directories from filename", [this]()
		{
			SentryAttachment->InitializeWithData({ 0x01 }, TEXT("logs/data.bin"));

			TestEqual("Filename", SentryAttachment->GetFilename(), TEXT("data.bin"));
		});
	});

	Describe("Path attachment", [this]()
	{
		It("should persist its values", [this]()
		{
			const FString TestPath = TEXT("/tmp/sentry/file.txt");
			const FString TestFilename = TEXT("custom.txt");
			const FString TestContentType = TEXT("text/plain");

			SentryAttachment->InitializeWithPath(TestPath, TestFilename, TestContentType);

			TestEqual("Path", SentryAttachment->GetPath(), TestPath);
			TestEqual("Filename", SentryAttachment->GetFilename(), TestFilename);
			TestEqual("Content type", SentryAttachment->GetContentType(), TestContentType);
			TestEqual("Data", SentryAttachment->GetData().Num(), 0);
		});

		It("should use default content type if not specified", [this]()
		{
			SentryAttachment->InitializeWithPath(TEXT("/tmp/sentry/file.txt"), TEXT("file.txt"));

			TestEqual("Content type", SentryAttachment->GetContentType(), TEXT("application/octet-stream"));
		});

		It("should use file name from path if filename is empty", [this]()
		{
			SentryAttachment->InitializeWithPath(TEXT("/tmp/sentry/file.txt"), TEXT(""));

			TestEqual("Filename", SentryAttachment->GetFilename(), TEXT("file.txt"));
		});

		It("should strip directories from filename", [this]()
		{
			SentryAttachment->InitializeWithPath(TEXT("/tmp/sentry/file.txt"), TEXT("logs/custom.txt"));

			TestEqual("Filename", SentryAttachment->GetFilename(), TEXT("custom.txt"));
		});
	});

	Describe("Attachment created via library", [this]()
	{
		It("should normalize filename of data attachment", [this]()
		{
			USentryAttachment* Attachment = USentryLibrary::CreateSentryAttachmentWithData({ 0x01 }, TEXT("logs/data.bin"), TEXT("application/octet-stream"));

			TestEqual("Filename", Attachment->GetFilename(), TEXT("data.bin"));
		});

		It("should normalize filename of path attachment", [this]()
		{
			USentryAttachment* Attachment = USentryLibrary::CreateSentryAttachmentWithPath(TEXT("/tmp/sentry/file.txt"), TEXT(""), TEXT("text/plain"));

			TestEqual("Filename", Attachment->GetFilename(), TEXT("file.txt"));
			TestEqual("Path", Attachment->GetPath(), TEXT("/tmp/sentry/file.txt"));
		});
	});
}

#endif
