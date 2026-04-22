#!/usr/bin/env bash
# Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
# shellcheck disable=SC2086

set -eo pipefail

STDLIB_ABC="plugins/ets/etsstdlib.abc"
PAOC_OUTPUT="etsstdlib.an"
for ARGUMENT in "$@"; do
case "$ARGUMENT" in
    --binary-dir=*)
    PANDA_BINARY_ROOT="${ARGUMENT#*=}"
    ;;
    --target-arch=*)
    TARGET_ARCH="--compiler-cross-arch=${ARGUMENT#*=}"
    ;;
    --paoc-mode=*)
    PAOC_MODE="--paoc-mode=${ARGUMENT#*=}"
    ;;
    --prefix=*)
    PANDA_RUN_PREFIX="${ARGUMENT#*=}"
    ;;
    -compiler-options=*)
    OPTIONS="${ARGUMENT#*=}"
    ;;
    -paoc-output=*)
    PAOC_OUTPUT="${ARGUMENT#*=}"
    ;;
    -paoc-regex=*)
    PAOC_REGEX="--compiler-regex=${ARGUMENT#*=}"
    ;;
    -stdlib-abc=*)
    STDLIB_ABC="${ARGUMENT#*=}"
    ;;
    *)
    echo "Unexpected argument: '${ARGUMENT}'"
    exit 1
    ;;
esac
done

if [ -z "$PANDA_BINARY_ROOT" ]; then
    echo "Please set PANDA_BINARY_ROOT"
    exit 1
fi

${PANDA_RUN_PREFIX} ${PANDA_BINARY_ROOT}/bin/ark_aot --boot-panda-files=${PANDA_BINARY_ROOT}/${STDLIB_ABC} \
                    --paoc-panda-files=${PANDA_BINARY_ROOT}/${STDLIB_ABC}  \
                    --compiler-ignore-failures=false $PAOC_MODE $PAOC_REGEX --load-runtimes="ets" \
                    $TARGET_ARCH ${OPTIONS}  --paoc-output=${PAOC_OUTPUT}
