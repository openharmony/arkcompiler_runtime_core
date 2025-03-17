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

class A {
  meth(p: number): number { return p + 42.0 }
}

class B {
  meth(p: number): number { return p + 33.0 }
}

let u1: A|B = new A
let u2: A|B = new B

function main() {
  assertEQ(u1.meth(1.0), 43.0);
  assertEQ(u2.meth(2.0), 35.0);
}
