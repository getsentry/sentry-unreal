# Threads and lifetime

## Threads in the plugin

| Thread | Code | Notes |
|--------|------|-------|
| Game thread | `USentrySubsystem`, integrations, settings, game-state sampling | Init, Close, re-init, and `UObject` creation normally happen here |
| Render thread | `SessionReplay/SentryBackBufferCapture.*` (`*_RenderThread` methods) | Captures back buffers into pooled `FSentryVideoFrame`s and hands them to the encoder |
| Encoder thread | `SessionReplay/SentryVideoEncoder.*` (`FRunnable`, `QueueLock`, `WakeEvent`) | Drains the frame queue and writes MP4 fragments |
| Rotation thread | `SessionReplay/SentrySessionReplayRecorder.*` (`RingLock`, `RotationWake`) | Rotates replay files. `Shutdown()` joins it before the encoder is reset |
| Hang watcher | `HangTracking/SentryHangWatcher.*` | Heartbeat callbacks bound with `BindRaw` |
| Game-state writer | `GameState/SentryGameStateFileWriter.*` (`QueueLock`, `WriteLock`) | Writes `game_state.jsonl` |
| SDK callback threads | `beforeSend`, `beforeBreadcrumb`, `beforeLog`, `beforeMetric`, traces sampler | Can run on any thread, including during GC or post-load. A callback that creates `UObject`s must go through `SentryCallbackUtils::IsCallbackSafeToRun()` (GC and post-load, #850) and `TSentryCallbackGuard<T>` (re-entry) |
| Output devices | `FSentryOutputDevice`, `FSentryErrorOutputDevice`, ensure (`FCoreDelegates::OnHandleSystemEnsure`) and assert (`OnAssert`) hooks | Run on whichever thread logs, ensures, or asserts. `FSentryOutputDevice::CanBeUsedOnAnyThread()` returns true, so a `UE_LOG` on a worker thread becomes a breadcrumb or log on that thread, and `beforeBreadcrumb`/`beforeLog` run there |
| Crash handler | `OnCrash` in `GenericPlatformSentrySubsystem.cpp` and its overrides | Runs while the process is crashing. Uses pre-allocated `PooledCrashEvent`/`PooledCrashHint` |
