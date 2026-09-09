# Copyright (c) 2026 Sentry. All Rights Reserved.

import unreal

# Agent skills are provided by the AI toolset registry, an optional editor-only engine plugin.
# When it isn't enabled the base class and its decorator module don't exist, so skip registration
# entirely rather than failing the import at editor startup.
if hasattr(unreal, 'AgentSkill'):
    from sentry_unreal import skills  # noqa: F401 - importing registers the skills
