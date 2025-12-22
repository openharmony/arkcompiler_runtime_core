#!/bin/bash
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

# Usage: llvm-config.sh <output_file> <llvm-config> <arg1> <arg2> ...

outfile=$1
if [ -z "$outfile" ]; then
    echo "missing output_file"
    exit 1
fi
shift

llvm_config=$1
if [ -z "$llvm_config" ]; then
    echo "missing llvm-config"
    exit 1
else
    llvm_config=$(type -p "$llvm_config")
    if [ -z "$llvm_config" ]; then
        echo "cannot find or execute $llvm_config"
        exit 1
    fi
fi
shift

set -e
$llvm_config "$@" > "$outfile"
