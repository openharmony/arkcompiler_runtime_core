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
interface TestInterface {
    property: number
}

function syntheicClassDemanglingTest() {
    let o: TestInterface = {property: 1}
    assertEQ(JSON.stringify(o), '{"property":1}')
}

function syntheicClassManglingTest() {
    let o: TestInterface = {property: 1}
    let o2 = JSON.parse<TestInterface>('{"property":2}', Type.of(o));
    assertEQ(JSON.stringify(o2), '{"property":2}')
}

function main(): int {
    const suite = new ArkTestsuite("DeMangling test for interface")
    suite.addTest("Demangling for object literal implements interface", syntheicClassDemanglingTest)
    suite.addTest("Mangling for object literal implements interface", syntheicClassManglingTest)
    return suite.run()
}
