/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import {doubleNegInf, doubleNaN} from "std/math/consts";

function main(): int {
    let roundTestsuite = new arktest.ArkTestsuite("Math.log10 int exception");
    roundTestsuite.addTest("test1", test1NegInf);
    roundTestsuite.addTest("test2", test2NaN);
    return roundTestsuite.run();
}

function test1NegInf() {
    // Test with doubleInf has already added in /static_core/plugins/ets/tests/stdlib-templates/std/math/list.std_math_log.yaml
    let expectedException = new ArgumentOutOfRangeException("Integer argument for log10 should be positive")
    const v : int = (doubleNegInf).toInt();
    arktest.expectException((): void => { log10(v) }, expectedException);
}

function test2NaN() {
    let expectedException = new ArgumentOutOfRangeException("Integer argument for log10 should be positive")
    const v : int = (doubleNaN).toInt();
    arktest.expectException((): void => { log10(v) }, expectedException);
}
