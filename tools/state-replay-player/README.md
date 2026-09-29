# Sentry State Replay — local player (v0, experimental)

State-based session replay for Unreal: instead of recording video, the SDK
samples a compact, structured snapshot of gameplay state each frame and streams
it to a local **newline-delimited JSON (JSONL)** file. It's cheap enough to run
continuously in dev, trivially parseable by tools and AI agents, and replayable
in the bundled offline viewer.

This is a **local developer/debugging tool**. It does not upload anything and
does not touch the crash pipeline (that's a later phase).

## SDK feature

- `USentryStateReplaySettings` (Project Settings → Plugins → *Sentry State Replay*)
- `USentryStateReplaySubsystem` (`UGameInstanceSubsystem`) — samples the local
  pawn transform, camera POV, active UMG widgets, and any tracked actors.

Enable via config:

```ini
[/Script/Sentry.SentryStateReplaySettings]
bEnabled=True
bAutoStart=True
SampleRateHz=15
bCaptureUI=True
OutputSubdir=StateReplays
```

Sessions are written to `<Project>/Saved/<OutputSubdir>/session-<utc>-<label>.jsonl`.

### API (C++ / Blueprint)

```cpp
if (auto* SR = USentryStateReplaySubsystem::Get(this))
{
    SR->StartSession(TEXT("boss_fight"));        // if not auto-starting
    SR->AddTrackedActor(Enemy, Enemy->GetName(), TEXT("enemy"));
    SR->AddEvent(TEXT("combat"), TEXT("turret_fire"), { {TEXT("turret"), TEXT("1")} });
    SR->StopSession();
}
```

`AddTrackedActor` streams that actor's transform into every frame; the subsystem
auto-emits an `actor/despawn` event when the actor is destroyed.

## JSONL format (schema 1)

One `header` line, then interleaved `frame` (sampled) and `event` (pushed) lines.

```jsonc
{"type":"header","schema":1,"game":"SentryTower","map":"Main","engine":"5.8...","sampleRateHz":15,"units":"cm","coordinateSystem":"unreal-left-handed-z-up","rotationOrder":"pitch_yaw_roll_deg", ...}
{"type":"frame","t":0.84,"player":{"loc":[x,y,z],"rot":[pitch,yaw,roll]},"camera":{"loc":[...],"rot":[...],"fov":90.0},"actors":[{"id":"Enemy_0","tag":"enemy","loc":[...],"rot":[...]}],"ui":["W_HUD_C"]}
{"type":"event","t":0.80,"category":"actor","name":"spawn","data":{"id":"Enemy_0","tag":"enemy"}}
```

Units are centimeters; rotations are degrees `[pitch, yaw, roll]`; the coordinate
system is Unreal's left-handed, Z-up.

## Local player

Open `index.html` in any browser (no server) and drop a `session-*.jsonl` file
onto it — or use the file picker. A product-style dashboard driven by one shared
timeline (play / scrub / speed / keyboard):

- **World** — 3D reconstruction with low-poly stand-in assets (tower, enemies),
  ground grid, camera-frustum gizmo, motion trails, orbit controls, and a
  **POV** button that reproduces the player's captured camera. Falls back to a
  2D top-down view if WebGL/three.js is unavailable.
- **Game screen** — letterboxed UMG reconstruction: widget rects colored by
  type, text in place, hover/disabled state, progress-bar fills, live cursor,
  click pings.
- **Event timeline** — gameplay + UI events (click to seek).
- **Telemetry** — FPS / entity-count sparklines + event ticks (click to seek).
- **✨ AI digest** — one-click deterministic session summary to copy to an agent.

```bash
open tools/state-replay-player/index.html
```

For a one-click, self-contained view (three.js + session inlined, auto-renders):

```bash
./tools/state-replay-player/bake-view.sh [path/to/session.jsonl]
```

### 3D view dependency

The 3D world uses [three.js](https://threejs.org). It is **not committed**
(`.gitignore`d); fetch it once next to `index.html` — the viewer degrades to the
2D top-down view without it:

```bash
curl -fsSL -o tools/state-replay-player/three.min.js \
  https://unpkg.com/three@0.160.0/build/three.min.js
```
