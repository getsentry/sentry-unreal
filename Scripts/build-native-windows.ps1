param([string] $SentryNativeRoot,[string] $SentryArtifactsDestination)

Remove-Item ${SentryArtifactsDestination}/bin/*
Remove-Item ${SentryArtifactsDestination}/lib/*
Remove-Item ${SentryArtifactsDestination}/include/*

Push-Location $SentryNativeRoot

cmake -B build -D SENTRY_BACKEND=crashpad -D SENTRY_SDK_NAME=sentry.native.unreal
cmake --build build --target sentry --config RelWithDebInfo --parallel
cmake --build build --target crashpad_handler --config Release --parallel

Pop-Location

Copy-Item ${SentryNativeRoot}/build/RelWithDebInfo/sentry.lib -Destination ${SentryArtifactsDestination}/lib/sentry.lib
Copy-Item ${SentryNativeRoot}/build/RelWithDebInfo/sentry.dll -Destination ${SentryArtifactsDestination}/bin/sentry.dll
Copy-Item ${SentryNativeRoot}/build/RelWithDebInfo/sentry.pdb -Destination ${SentryArtifactsDestination}/bin/sentry.pdb
Copy-Item ${SentryNativeRoot}/build/crashpad_build/handler/Release/crashpad_handler.exe -Destination ${SentryArtifactsDestination}/bin/crashpad_handler.exe
Copy-Item ${SentryNativeRoot}/include/sentry.h -Destination ${SentryArtifactsDestination}/include/sentry.h

