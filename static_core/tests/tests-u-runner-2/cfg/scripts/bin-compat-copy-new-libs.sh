#!/bin/bash
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

# Copy <test>.lib<N>.new.ets -> <test>.lib<N>.ets for all new lib versions.
# Libs without a .new.ets counterpart are left untouched.
#
# Usage: bin-compat-copy-new-libs.sh <gen-dir>/<test-id-name>
# Example: bin-compat-copy-new-libs.sh /path/gen/23.binary_compatibility/test

set -e

TEST_ID_PATH="$1"

for new_lib in "${TEST_ID_PATH}".libr*.new.ets; do
    [ -f "$new_lib" ] || continue
    old_lib="${new_lib%.new.ets}.ets"
    cp "$old_lib" "${old_lib}.orig"
    cp "$new_lib" "$old_lib"
done
