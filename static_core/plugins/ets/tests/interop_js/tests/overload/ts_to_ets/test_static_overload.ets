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

import { OverloadClass, instance } from 'testDecl';

function testInstanceFnOverload() {
    arktest.assertEQ(instance.overloadFn(1), 'number');
    arktest.assertEQ(instance.overloadFn('2'), 'string');
    arktest.assertEQ(instance.overloadFn(true), 'boolean');
    arktest.assertEQ(instance.overloadFn(1, '2'), 'number-string');
    arktest.assertEQ(instance.overloadFn('2', 1), 'string-number');
    arktest.assertEQ(instance.overloadFn(1, 2, 3), 'number-number-number');
    arktest.assertEQ(instance.overloadFn('2', '3', '4'), 'string-string-string');
}

function testStaticFnOverload() {
    arktest.assertEQ(OverloadClass.overloadStaticFn(1), 'number');
    arktest.assertEQ(OverloadClass.overloadStaticFn('2'), 'string');
    arktest.assertEQ(OverloadClass.overloadStaticFn(true), 'boolean');
    arktest.assertEQ(OverloadClass.overloadStaticFn(1, '2'), 'number-string');
    arktest.assertEQ(OverloadClass.overloadStaticFn('2', 1), 'string-number');
    arktest.assertEQ(OverloadClass.overloadStaticFn(1, 2, 3), 'number-number-number');
    arktest.assertEQ(OverloadClass.overloadStaticFn('2', '3', '4'), 'string-string-string');
}

function testAll(): void {
    testInstanceFnOverload();
    testStaticFnOverload();
}
