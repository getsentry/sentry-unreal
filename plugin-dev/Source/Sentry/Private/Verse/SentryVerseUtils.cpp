// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Verse/SentryVerseUtils.h"

#include "Misc/EngineVersionComparison.h"

#if !UE_VERSION_OLDER_THAN(6, 0, 0)

#include "Algo/Reverse.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

FString FSentryVerseUtils::GetUserMessage(const FString& Message)
{
	const int32 PrefixLength = FCString::Strlen(UserMessagePrefix);
	if (Message.Len() > PrefixLength && Message.StartsWith(UserMessagePrefix, ESearchCase::CaseSensitive) && Message.EndsWith(TEXT("'"), ESearchCase::CaseSensitive))
	{
		return Message.Mid(PrefixLength, Message.Len() - PrefixLength - 1);
	}

	return Message;
}

TArray<FSentryScriptStackFrame> FSentryVerseUtils::ParseCallstack(const FString& RuntimeErrorText)
{
	TArray<FString> Lines;
	RuntimeErrorText.ParseIntoArrayLines(Lines);

	TArray<FSentryScriptStackFrame> Frames;
	for (const FString& Line : Lines)
	{
		FSentryScriptStackFrame Frame;
		if (ParseFrameLine(Line, Frame))
		{
			Frames.Add(MoveTemp(Frame));
		}
	}

	Algo::Reverse(Frames);
	return Frames;
}

void FSentryVerseUtils::AddSourceContext(TArray<FSentryScriptStackFrame>& Frames)
{
	for (FSentryScriptStackFrame& Frame : Frames)
	{
		if (Frame.Filename.IsEmpty() || Frame.LineNumber <= 0)
		{
			continue;
		}

		const TArray<FString>* Lines = GetSourceLines(Frame.Filename);
		if (!Lines || Frame.LineNumber > Lines->Num())
		{
			continue;
		}

		const int32 LineIndex = Frame.LineNumber - 1;
		Frame.ContextLine = (*Lines)[LineIndex];

		for (int32 Index = FMath::Max(0, LineIndex - SourceContextLines); Index < LineIndex; ++Index)
		{
			Frame.PreContext.Add((*Lines)[Index]);
		}

		for (int32 Index = LineIndex + 1; Index < FMath::Min(Lines->Num(), LineIndex + 1 + SourceContextLines); ++Index)
		{
			Frame.PostContext.Add((*Lines)[Index]);
		}
	}
}

bool FSentryVerseUtils::ParseFrameLine(const FString& Line, FSentryScriptStackFrame& OutFrame)
{
	FString Function;
	FString Location;
	if (!Line.Split(SourceMarker, &Function, &Location))
	{
		return false;
	}

	ParseFunctionName(Function.TrimStartAndEnd(), OutFrame);

	Location.TrimEndInline();
	Location.RemoveFromEnd(TEXT(")"));

	// Location has the form "<path>(<row>,<col>, <endRow>,<endCol>)"
	int32 SpanStart = INDEX_NONE;
	if (Location.EndsWith(TEXT(")")) && Location.FindLastChar(TEXT('('), SpanStart))
	{
		TArray<FString> Numbers;
		Location.Mid(SpanStart + 1, Location.Len() - SpanStart - 2).ParseIntoArray(Numbers, TEXT(","));
		if (Numbers.Num() == 4)
		{
			OutFrame.Filename = Location.Left(SpanStart);
			OutFrame.LineNumber = FCString::Atoi(*Numbers[0].TrimStartAndEnd());
			OutFrame.ColumnNumber = FCString::Atoi(*Numbers[1].TrimStartAndEnd());
			return true;
		}
	}

	OutFrame.Filename = Location;
	return true;
}

void FSentryVerseUtils::ParseFunctionName(const FString& FullName, FSentryScriptStackFrame& OutFrame)
{
	FString Name = FullName;

	// Qualified names have the form "(<module path>:)<name>(<parameter types>)"
	const int32 QualifierEnd = Name.StartsWith(TEXT("(")) ? Name.Find(TEXT(":)")) : INDEX_NONE;
	if (QualifierEnd != INDEX_NONE)
	{
		OutFrame.Module = Name.Mid(1, QualifierEnd - 1);
		Name.RightChopInline(QualifierEnd + 2);
	}

	int32 ParametersStart = INDEX_NONE;
	if (Name.FindChar(TEXT('('), ParametersStart) && ParametersStart > 0)
	{
		Name.LeftInline(ParametersStart);
	}

	OutFrame.Function = Name;
	if (Name != FullName)
	{
		OutFrame.RawFunction = FullName;
	}
}

FString FSentryVerseUtils::ResolveSourcePath(const FString& VerseFilePath)
{
	if (FPaths::FileExists(VerseFilePath))
	{
		return VerseFilePath;
	}

	for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetEnabledPlugins())
	{
		const FPluginDescriptor& Descriptor = Plugin->GetDescriptor();
		if (!Descriptor.bCanContainVerse || Descriptor.VersePath.IsEmpty())
		{
			continue;
		}

		FString RelativePath;
		if (VerseFilePath.StartsWith(Descriptor.VersePath + TEXT("/")))
		{
			RelativePath = VerseFilePath.RightChop(Descriptor.VersePath.Len() + 1);
		}
		else if (FPaths::IsRelative(VerseFilePath))
		{
			RelativePath = VerseFilePath;
		}
		else
		{
			continue;
		}

		const FString CandidatePath = FPaths::Combine(Plugin->GetContentDir(), RelativePath);
		if (FPaths::FileExists(CandidatePath))
		{
			return CandidatePath;
		}
	}

	return FString();
}

const TArray<FString>* FSentryVerseUtils::GetSourceLines(const FString& VerseFilePath)
{
	static TMap<FString, TArray<FString>> SourceCache;

	if (const TArray<FString>* CachedLines = SourceCache.Find(VerseFilePath))
	{
		return CachedLines;
	}

	TArray<FString>& Lines = SourceCache.Add(VerseFilePath);

	const FString SourcePath = ResolveSourcePath(VerseFilePath);
	if (!SourcePath.IsEmpty())
	{
		FFileHelper::LoadFileToStringArray(Lines, *SourcePath);
	}

	return &Lines;
}

#endif
