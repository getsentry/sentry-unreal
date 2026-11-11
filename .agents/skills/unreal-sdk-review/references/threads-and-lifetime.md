# Threads and lifetime

## Threads in the plugin

| Thread | Code | Notes |
|--------|------|-------|
| Game thread | `USentrySubsystem`, integrations, settings, game-state sampling | Init, Close, re-init, and `UObject` creation normally happen here |
| Render thread | `SessionReplay/SentryBackBufferCapture.*` (`*_RenderThread` methods) | Captures back buffers and hands `FTextureRHIRef`s to the encoder |
| Encoder thread | `SessionReplay/SentryVideoEncoder.*` (`FRunnable`, `QueueLock`, `WakeEvent`) | Drains the frame queue and writes MP4 fragments |
| Rotation thread | `SessionReplay/SentrySessionReplayRecorder.*` (`RingLock`, `RotationWake`) | Rotates replay files. `Shutdown()` joins it before the encoder is reset |
| Hang watcher | `HangTracking/SentryHangWatcher.*` | Heartbeat callbacks bound with `BindRaw` |
| Game-state writer | `GameState/SentryGameStateFileWriter.*` (`QueueLock`, `WriteLock`) | Writes `game_state.jsonl` |
| SDK callback threads | `beforeSend`, `beforeBreadcrumb`, `beforeLog`, `beforeMetric`, traces sampler, ensure/assert hooks | Can run on any thread, including during GC or post-load |
| Crash handler | `OnCrash` in `GenericPlatformSentrySubsystem.cpp` and its overrides | Runs while the process is crashing. Uses pre-allocated `PooledCrashEvent`/`PooledCrashHint` |

## Rules this code relies on

- Every shared container or flag has one lock. Find the lock the writer holds and check that every reader holds it too, including size and emptiness checks before a wait.
- `FEvent` waits must not hold a lock the waking thread needs.
- `FTextureRHIRef` pool slots are reused only when `GetRefCount() <= 1`. A local array that keeps copies alive across a wait blocks reuse. A slot reused while the encoder still holds it shows the wrong frame.
- Delegates bound with `AddRaw`/`BindRaw` to events fired from other threads (ensure, assert, hang heartbeat) can still be executing when `Teardown` unbinds them. The owner must not be destroyed until in-flight calls finish. Raw bindings to game-thread-only delegates (map load, game-state class change) that are removed on the game thread are fine.
- A `SWindow`, widget, actor, or `UObject` kept past the current call needs a `TWeakPtr`/`TWeakObjectPtr` or a lifetime the owner controls.
- `UObject` members that must survive GC are `UPROPERTY()` or otherwise referenced. A raw `UObject*` in a non-`UObject` class is collected.
