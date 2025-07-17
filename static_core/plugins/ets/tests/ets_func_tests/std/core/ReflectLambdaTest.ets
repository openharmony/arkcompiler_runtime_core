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

class ClassA {}

interface A {
    a: (index: number, str:ClassA) => [number, number]
}

function testReflectLamdaTest() {
    let obja: A = { a:(index: number, str:ClassA) => {return [1, 2] as [number, number]} }
    let objb: A = { a:(index: number, str:ClassA) => {return [3, 4] as [number, number]} }
    let ret = Reflect.set(obja, "a", objb.a)
    arktest.assertEQ(ret, true);
}

function main(): int {
   const suite = new arktest.ArkTestsuite("Reflect.LambdaTest");
   suite.addTest("Test testReflectLamdaTest", testReflectLamdaTest);
   return suite.run()
}