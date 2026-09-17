# Copyright (c) 2026 Sentry. All Rights Reserved.

try:
    from sentry_unreal import toolsets
    from sentry_unreal import skills  # noqa: F401 - importing registers the skills
except ImportError:
    pass
else:
    toolsets._registration.register()
