#!/usr/bin/env bash
# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

DIR=$(cd $(dirname "$0") && pwd)

if [ $# -ge 3 ]; then
    TEST=$1
    PANDA_BUILD=$2
    BIN_TEST_DIR=$3

    PANDA_RUN_PREFIX="$4 $5 $6"

    echo "Running $DIR/.../pa/${TEST}"

    $PANDA_RUN_PREFIX "$PANDA_BUILD/bin/ark_asm" "$DIR/../pa/$TEST" "$BIN_TEST_DIR/$TEST.abc"
    LD_LIBRARY_PATH="$BIN_TEST_DIR/lib" $PANDA_RUN_PREFIX "$PANDA_BUILD/bin/ark" --boot-panda-files="$PANDA_BUILD/plugins/ets/etsstdlib.abc" --load-runtimes=ets --run-gc-in-place --gc-type=epsilon "$BIN_TEST_DIR/$TEST.abc" EtsNapiTests::main

else
    echo "Usage test.sh <test_name> <panda_build_dir> <dir_to_save_bins> <PANDA_RUN_PREFIX>"
fi
