#!/usr/bin/env bash
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Fetch curated external TypeScript projects used as benchmark fixtures.
# Each project is pinned to a release tag for reproducibility.
#
# Usage: npm run bench:fetch
#
# Idempotent: existing clones are skipped. Pass FORCE=1 to re-clone.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXT="$SCRIPT_DIR/../fixtures/external"
mkdir -p "$EXT"

# name|repo url|git ref|subdir to feed into declgen
PROJECTS=(
    "zod|https://github.com/colinhacks/zod.git|v3.23.8|src"
    "rxjs|https://github.com/ReactiveX/rxjs.git|7.8.1|src"
    "typeorm|https://github.com/typeorm/typeorm.git|0.3.20|src"
    "nestjs|https://github.com/nestjs/nest.git|v10.4.5|packages"
)

function clone_one() {
    local name="$1" url="$2" ref="$3"
    local dest="$EXT/$name"
    if [[ -d "$dest/.git" && "${FORCE:-0}" != "1" ]]; then
        echo "[skip ] $name (already cloned)"
        return 0
    fi
    if [[ -d "$dest" && "${FORCE:-0}" == "1" ]]; then
        rm -rf "$dest"
    fi
    echo "[clone] $name @ $ref"
    git clone --depth 1 --branch "$ref" --quiet "$url" "$dest"
}

function install_one() {
    local name="$1"
    local dest="$EXT/$name"
    if [[ -d "$dest/node_modules" ]]; then
        echo "[skip ] $name npm install (node_modules present)"
        return 0
    fi
    if [[ ! -f "$dest/package.json" ]]; then
        return 0
    fi
    echo "[deps ] $name (npm install)"
    (cd "$dest" && npm install --omit=optional --no-audit --no-fund --ignore-scripts --loglevel=error) \
        || echo "[warn ] npm install failed for $name; benchmark may run with unresolved imports"
}

for entry in "${PROJECTS[@]}"; do
    IFS='|' read -r name url ref _subdir <<<"$entry"
    clone_one "$name" "$url" "$ref"
done

for entry in "${PROJECTS[@]}"; do
    IFS='|' read -r name _url _ref _subdir <<<"$entry"
    install_one "$name"
done

echo "Done. External fixtures available under: $EXT"
