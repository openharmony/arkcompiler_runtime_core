#!/usr/bin/env bash
# Copyright (c) 2025 Huawei Device Co., Ltd.
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

set -eo pipefail

if [ $# -ne 3 ]; then
    echo "Usage: $0 <STD_DIR> <ESCOMPACT_DIR> <TARGET_DIR>"
    exit 1
fi

STD="$1"
ESCOMPACT="$2"
TARGET="$3"

if [ ! -d "$TARGET" ]; then
    mkdir -p "$TARGET"
fi

cp -r -f "$STD" "$TARGET"
cp -r -f "$ESCOMPACT" "$TARGET"
