/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

const NULL_STR: String | null = null;
let str = 'abc';
function testLineStringEquals() {
    arktest.assertNE(str, NULL_STR, 'String must not be equal to null');
    arktest.assertNE(NULL_STR, str, 'Null string must not be equal to non-null one');
    arktest.assertEQ(NULL_STR, NULL_STR, 'Null values of type String must be equal');
    arktest.assertEQ(str, str, 'String must be equal to itself');
    arktest.assertEQ(str, 'abc', "String must be equal to 'abc'");
}

function testTreeStringEquals() {
    let treeStr = str.concat('defghijklmnopqrstuvwxyz');
    arktest.assertNE(treeStr, NULL_STR, 'TreeString must not be equal to null');
    arktest.assertNE(NULL_STR, treeStr, 'Null string must not be equal to non-null one');
    arktest.assertEQ(treeStr, treeStr, 'TreeString must be equal to itself');
    arktest.assertEQ(treeStr, 'abcdefghijklmnopqrstuvwxyz', "TreeString must be equal to 'abcdefghijklmnopqrstuvwxyz'");
}

function testSlicedStringEquals() {
    let treeStr = str.concat('defghijklmnopqrstuvwxyz');
    let slicedStr = treeStr.substring(1, 17);  // "bcdefghijklmnopq"
    arktest.assertNE(slicedStr, NULL_STR, 'SlicedString must not be equal to null');
    arktest.assertNE(NULL_STR, slicedStr, 'Null string must not be equal to non-null one');
    arktest.assertEQ(slicedStr, slicedStr, 'SlicedString must be equal to itself');
    arktest.assertEQ(slicedStr, 'bcdefghijklmnopq', "SlicedString must be equal to 'bcde'");
}

function main(): int {
    let suite = new arktest.ArkTestsuite('StringEquals');
    suite.addTest('testLineStringEquals', testLineStringEquals);
    suite.addTest('testTreeStringEquals', testTreeStringEquals);
    suite.addTest('testSlicedStringEquals', testSlicedStringEquals);
    return suite.run();
}
