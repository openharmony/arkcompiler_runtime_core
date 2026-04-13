#!/usr/bin/env bash

# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd -- "$SCRIPT_DIR/../.." && pwd)"
BOOTSTRAP_PY="$REPO_ROOT/src/arkts_migration_visualizer/deps/bootstrap_impl.py"
PYTHON_VERSION_CHECK='import sys; raise SystemExit(0 if (3, 10) <= sys.version_info[:2] <= (3, 12) else 1)'

for launcher_var in BOOTSTRAP_PYTHON HAPRAY_PYTHON; do
    candidate="${!launcher_var:-}"
    [[ -n "$candidate" ]] || continue
    if [[ "$candidate" == */* ]]; then
        [[ -x "$candidate" ]] || continue
    elif ! command -v "$candidate" >/dev/null 2>&1; then
        continue
    fi
    if "$candidate" -c "$PYTHON_VERSION_CHECK" >/dev/null 2>&1; then
        exec "$candidate" "$BOOTSTRAP_PY" "$@"
    fi
done

for candidate in \
    "$REPO_ROOT/.py311/bin/python" \
    "$REPO_ROOT/.py311/Scripts/python.exe" \
    python3.12 \
    python3.11 \
    python3.10 \
    python3 \
    python
do
    if [[ "$candidate" == */* ]]; then
        [[ -x "$candidate" ]] || continue
    else
        command -v "$candidate" >/dev/null 2>&1 || continue
    fi
    if "$candidate" -c "$PYTHON_VERSION_CHECK" >/dev/null 2>&1; then
        exec "$candidate" "$BOOTSTRAP_PY" "$@"
    fi
done

if command -v py >/dev/null 2>&1; then
    for version_flag in -3.12 -3.11 -3.10 -3; do
        if py "$version_flag" -c "$PYTHON_VERSION_CHECK" >/dev/null 2>&1; then
            exec py "$version_flag" "$BOOTSTRAP_PY" "$@"
        fi
    done
fi

echo "No suitable Python 3.10-3.12 launcher found. Set BOOTSTRAP_PYTHON or HAPRAY_PYTHON first." >&2
exit 1
