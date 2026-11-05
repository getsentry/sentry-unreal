// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryTests.h"
#include "SentryVariant.h"

#include "GameState/SentryGameStateJsonWriter.h"
#include "GameState/SentryGameStateRecorder.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryGameStateSpec, "Sentry.SentryGameState", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	TArray<FString> SnapshotLines(const FSentryGameStateRecorder& Recorder) const
	{
		TArray<uint8> Snapshot = Recorder.BuildSnapshot();
		const FUTF8ToTCHAR Converted(reinterpret_cast<const ANSICHAR*>(Snapshot.GetData()), Snapshot.Num());
		const FString Text(Converted.Length(), Converted.Get());

		TArray<FString> Lines;
		Text.ParseIntoArray(Lines, TEXT("\n"));
		return Lines;
	}

	TSharedPtr<FJsonObject> ParseObject(const FString& Json) const
	{
		TSharedPtr<FJsonObject> Object;
		FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Object);
		return Object;
	}
END_DEFINE_SPEC(SentryGameStateSpec)

void SentryGameStateSpec::Define()
{
	Describe("Json writer", [this]()
	{
		It("should write valid nested json", [this]()
		{
			FSentryGameStateJsonWriter Writer;
			Writer.BeginObject()
				.Write(TEXT("type"), TEXT("frame"))
				.Write(TEXT("t"), 1.5)
				.Write(TEXT("loc"), FVector(1.0f, -2.25f, 3.333f))
				.BeginArray(TEXT("actors"))
				.BeginObject()
				.Write(TEXT("id"), TEXT("a"))
				.EndObject()
				.BeginObject()
				.Write(TEXT("id"), TEXT("b"))
				.EndObject()
				.EndArray()
				.Write(TEXT("paused"), false)
				.EndObject();

			TestEqual("Output", Writer.GetOutput(),
				TEXT("{\"type\":\"frame\",\"t\":1.5,\"loc\":[1,-2.25,3.33],\"actors\":[{\"id\":\"a\"},{\"id\":\"b\"}],\"paused\":false}"));
			TestTrue("Parses as json", ParseObject(Writer.GetOutput()).IsValid());
		});

		It("should escape strings", [this]()
		{
			FSentryGameStateJsonWriter Writer;
			Writer.BeginObject().Write(TEXT("text"), FString(TEXT("quote\" slash\\ line\n tab\t \x01"))).EndObject();

			TSharedPtr<FJsonObject> Parsed = ParseObject(Writer.GetOutput());
			TestTrue("Parses as json", Parsed.IsValid());
			if (Parsed.IsValid())
			{
				TestEqual("Round-trip", Parsed->GetStringField(TEXT("text")), FString(TEXT("quote\" slash\\ line\n tab\t \x01")));
			}
		});

		It("should format numbers compactly", [this]()
		{
			TestEqual("Integral", FSentryGameStateJsonWriter::FormatNumber(42.0, 3), TEXT("42"));
			TestEqual("Trailing zeros", FSentryGameStateJsonWriter::FormatNumber(0.5, 3), TEXT("0.5"));
			TestEqual("Rounding", FSentryGameStateJsonWriter::FormatNumber(1.23456, 2), TEXT("1.23"));
			TestEqual("Negative zero", FSentryGameStateJsonWriter::FormatNumber(-0.0001, 2), TEXT("0"));
			TestEqual("NaN", FSentryGameStateJsonWriter::FormatNumber(NAN, 2), TEXT("null"));
		});

		It("should write variants", [this]()
		{
			TMap<FString, FSentryVariant> Map;
			Map.Add(TEXT("level"), 3);

			TArray<FSentryVariant> Array;
			Array.Add(TEXT("sword"));
			Array.Add(true);

			FSentryGameStateJsonWriter Writer;
			Writer.BeginObject()
				.Write(TEXT("int"), FSentryVariant(7))
				.Write(TEXT("float"), FSentryVariant(0.25f))
				.Write(TEXT("bool"), FSentryVariant(true))
				.Write(TEXT("string"), FSentryVariant(TEXT("boss")))
				.Write(TEXT("array"), FSentryVariant(Array))
				.Write(TEXT("map"), FSentryVariant(Map))
				.Write(TEXT("empty"), FSentryVariant())
				.EndObject();

			TestEqual("Output", Writer.GetOutput(),
				TEXT("{\"int\":7,\"float\":0.25,\"bool\":true,\"string\":\"boss\",\"array\":[\"sword\",true],\"map\":{\"level\":3},\"empty\":null}"));
		});
	});

	Describe("Recorder", [this]()
	{
		It("should write header before records", [this]()
		{
			FSentryGameStateRecorder Recorder(30.0, 1024 * 1024);
			Recorder.SetHeader(TEXT("{\"type\":\"header\"}"));
			Recorder.AddRecord(1.0, TEXT("{\"t\":1}"));
			Recorder.AddRecord(2.0, TEXT("{\"t\":2}"));

			const TArray<FString> Lines = SnapshotLines(Recorder);
			TestEqual("Number of lines", Lines.Num(), 3);
			if (Lines.Num() == 3)
			{
				TestEqual("Header", Lines[0], TEXT("{\"type\":\"header\"}"));
				TestEqual("First record", Lines[1], TEXT("{\"t\":1}"));
				TestEqual("Second record", Lines[2], TEXT("{\"t\":2}"));
			}
		});

		It("should drop records older than the window", [this]()
		{
			FSentryGameStateRecorder Recorder(10.0, 1024 * 1024);
			for (int32 Second = 0; Second <= 30; ++Second)
			{
				Recorder.AddRecord(Second, FString::Printf(TEXT("{\"t\":%d}"), Second));
			}

			TestEqual("Records within window", Recorder.GetNumRecords(), 11);

			const TArray<FString> Lines = SnapshotLines(Recorder);
			TestEqual("Oldest record", Lines.Num() > 0 ? Lines[0] : FString(), TEXT("{\"t\":20}"));
		});

		It("should drop oldest records when the size limit is reached", [this]()
		{
			const FString Record = TEXT("{\"t\":0,\"pad\":\"0123456789\"}");
			const int32 RecordSize = Record.Len() + 1;

			FSentryGameStateRecorder Recorder(1000.0, RecordSize * 5);
			for (int32 Index = 0; Index < 20; ++Index)
			{
				Recorder.AddRecord(Index, Record);
			}

			TestEqual("Records within size limit", Recorder.GetNumRecords(), 5);
			TestEqual("Size", Recorder.GetSizeBytes(), RecordSize * 5);
		});

		It("should keep the latest record if it exceeds the size limit", [this]()
		{
			FSentryGameStateRecorder Recorder(30.0, 8);
			Recorder.AddRecord(1.0, TEXT("{\"t\":1}"));
			Recorder.AddRecord(2.0, TEXT("{\"t\":2,\"pad\":\"0123456789\"}"));

			TestEqual("Records", Recorder.GetNumRecords(), 1);

			const TArray<FString> Lines = SnapshotLines(Recorder);
			TestEqual("Latest record", Lines.Num() > 0 ? Lines.Last() : FString(), FString(TEXT("{\"t\":2,\"pad\":\"0123456789\"}")));
		});

		It("should keep the header after reset", [this]()
		{
			FSentryGameStateRecorder Recorder(30.0, 1024 * 1024);
			Recorder.SetHeader(TEXT("{\"type\":\"header\"}"));
			Recorder.AddRecord(1.0, TEXT("{\"t\":1}"));
			Recorder.Reset();

			TestEqual("Records", Recorder.GetNumRecords(), 0);
			TestEqual("Size", Recorder.GetSizeBytes(), 0);

			const TArray<FString> Lines = SnapshotLines(Recorder);
			TestEqual("Only header", Lines.Num(), 1);
		});
	});
}

#endif
