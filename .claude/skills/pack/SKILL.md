---
name: pack
description: Package plugin and update file snapshot
---

Package the Sentry plugin and update the file snapshot. Use this after adding or removing files from the plugin.

1. Run the packaging script:

```bash
pwsh ./scripts/packaging/pack.ps1
```

2. Update the snapshot to accept the new file listing:

```bash
pwsh ./scripts/packaging/test-contents.ps1 accept
```
