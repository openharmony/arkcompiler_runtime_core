/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

import { BigIntHelper, bigIntHelperInstance } from 'testDecl';

function testBigIntNegateCallThisRange(): void {
    let result = bigIntHelperInstance.testBigInt2(-123n, 1n);
    arktest.assertEQ(result, -122n);
}

function testBigIntMultipleNegateCallThisRange(): void {
    let result = bigIntHelperInstance.testBigInt2(-456n, -789n);
    arktest.assertEQ(result, -1245n);
}

function testBigIntPositiveCallThisRange(): void {
    let result = bigIntHelperInstance.testBigInt2(100n, 200n);
    arktest.assertEQ(result, 300n);
}

function testAll(): void {
    testBigIntNegateCallThisRange();
    testBigIntMultipleNegateCallThisRange();
    testBigIntPositiveCallThisRange();
}
