#!/usr/bin/env bash
# Bake a state-replay JSONL session into a self-contained, auto-rendering HTML
# (three.js + session inlined) and open it.
# Usage: ./bake-view.sh [path/to/session.jsonl]   (defaults to newest in <demo>/Saved/StateReplays)
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
DEFAULT_DIR="$HOME/unreal/Saved/StateReplays"
SESSION="${1:-$(ls -t "$DEFAULT_DIR"/*.jsonl 2>/dev/null | head -1)}"
[ -n "${SESSION:-}" ] && [ -f "$SESSION" ] || { echo "No session found. Pass a .jsonl path."; exit 1; }
OUT="${SESSION%.jsonl}-view.html"
python3 - "$HERE/index.html" "$SESSION" "$OUT" "$HERE/three.min.js" <<'PY'
import sys, os
viewer, session, out = sys.argv[1:4]
threejs = sys.argv[4] if len(sys.argv) > 4 else None
html = open(viewer, encoding='utf-8').read()
if threejs and os.path.exists(threejs):
    tj = open(threejs, encoding='utf-8').read().replace('</', '<\\/')
    html = html.replace('<script src="three.min.js"></script>', '<script>'+tj+'</script>', 1)
data = open(session, encoding='utf-8').read().replace('</', '<\\/')
embed = '<script type="application/jsonl" id="embedded-session">\n'+data+'\n</script>\n<script>\n"use strict";'
html = html.replace('<script>\n"use strict";', embed, 1)
open(out, 'w', encoding='utf-8').write(html)
print("baked:", out, round(os.path.getsize(out)/1024), "KB")
PY
open "$OUT" 2>/dev/null || echo "Open manually: $OUT"
