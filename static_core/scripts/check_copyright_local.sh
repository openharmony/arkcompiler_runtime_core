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

#
# Check that copyright is properly updated in all files modified comparing to
# the base branch ('master' by default, can be overridden using BASE_BRANCH env variable)
#

CUR_YEAR=$(date +%Y)
EXIT_CODE=0

BASE_BRANCH=${BASE_BRANCH:-"master"}
ROOT_DIR=$(git rev-parse --show-toplevel)

while read file_to_check; do
    [ -f "${ROOT_DIR}/${file_to_check}" ] || continue
    if ! grep -iq 'Copyright (c) ' "${ROOT_DIR}/${file_to_check}"; then
        EXIT_CODE=1
        echo "No copyright in ${file_to_check}"
    elif ! grep -Eiq "Copyright \(c\).*([0-9]{4}-)?${CUR_YEAR} " "${ROOT_DIR}/${file_to_check}"; then
        EXIT_CODE=1
        echo "Wrong copyright year in ${file_to_check}, expected last year to be ${CUR_YEAR}"
    elif [[ $file_to_check == *.cpp || $file_to_check == *.chp || $file_to_check == *.c || $file_to_check == *.h || $file_to_check == *.groovy || $file_to_check == *.Jenkinsfile ]]; then
        if ! head -n 1 "${ROOT_DIR}/${file_to_check}" | grep -q '^/\*\*'; then
            EXIT_CODE=1
            echo "Copyright in ${file_to_check} should start with '/**' for cpp/c/groovy files"
        fi
    fi

done <<< $(git diff --name-only "${BASE_BRANCH}" | grep -E '\.cpp$|\.c$|\.hpp$|\.h$|\.js$|\.ts$|\.ets$|\.py$|\.sh$|\.groovy$|\.Jenkinsfile$')

num_checked=$(git diff --name-only "${BASE_BRANCH}" | grep -E '\.cpp$|\.c$|\.hpp$|\.h$|\.js$|\.ts$|\.ets$|\.py$|\.sh$|\.groovy$|\.Jenkinsfile$' | wc -l)
echo "Checked ${num_checked} file(s)"

exit $EXIT_CODE
