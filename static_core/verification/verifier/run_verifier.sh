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

set -e -o pipefail

function log() {
    if [ $# -eq 1 ]; then
        echo -e "$(basename "$0") : $1"
    fi
}

if [ "$#" -ne 2 ]; then
    log "no enough params"
    exit 1
fi

verifier=$1
stdlib=$2


if [[ -f "$verifier" && -f "$stdlib" ]]; then
    $("${verifier}" --boot-panda-files="${stdlib}"    --load-runtimes=ets    --verify-runtime-libraries=true)
else
    if [[ ! -f "$verifier" ]];  then
        log "\nno verifier $verifier found!\n"
    fi
    if [[ ! -f "$stdlib" ]]; then
        log "\nno stdlib $stdlib found!\n"
    fi
    exit 1
fi
