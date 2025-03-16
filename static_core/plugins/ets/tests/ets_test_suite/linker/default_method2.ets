/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

interface I1 {
    foo(): string { return "I1" }
}

interface I2 extends I1 {
    foo(): string { return "I2" }
}

abstract class A1 implements I2 {}
class A2 extends A1 {}

function main() {
    assertEQ(((x: I1) => x.foo())(new A2()), "I2");
    assertEQ(((x: I2) => x.foo())(new A2()), "I2");
}
