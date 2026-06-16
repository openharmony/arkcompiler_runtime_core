/*
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

function testStringBuilderConstructor1() {
    let builder = new StringBuilder();
    arktest.assertEQ(builder.toString(), '');
}

function testStringBuilderConstructor2() {
    let builder = new StringBuilder('hello');
    arktest.assertEQ(builder.toString(), 'hello');
}

function testStringBuilderConstructor3() {
    const chars:char[] = [c'A',c'B',c'C',c'D']
    let builder = new StringBuilder(chars)
    arktest.assertEQ(builder.toString(), 'ABCD');
}

function main() {
    let myTestsuite = new arktest.ArkTestsuite("StringBuilderTest");
    myTestsuite.addTest('testStringBuilderConstructor1', testStringBuilderConstructor1);
    myTestsuite.addTest('testStringBuilderConstructor2', testStringBuilderConstructor2);
    myTestsuite.addTest('testStringBuilderConstructor3', testStringBuilderConstructor3);
    return myTestsuite.run();   
}