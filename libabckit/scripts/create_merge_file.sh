#!/usr/bin/env bash
# Copyright (c) 2024 Huawei Device Co., Ltd.
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

set -e

base="$(basename $1)"
tmp="$(echo $base | sed 's/\..*//')"
tmp_file="/tmp/merge_$tmp.txt"
echo "$base;$tmp;esm;$base;entry" > $tmp_file
dir="$(dirname $1)"
modules_dir="$dir/modules"
if [ -d "$modules_dir" ]; then
    for file in "$modules_dir"/*; do
        name=$(basename "$file")
        tmp_module="$(echo $name | sed 's/\..*//')"
        echo "modules/$name;modules/$tmp_module;esm;modules/$name;entry" >> $tmp_file
    done
fi
/usr/bin/python3 "$(dirname $0)/../../../ets_runtime/test/quickfix/generate_merge_file.py" --input "$tmp_file" --output "$2" --prefix "$dir/"
