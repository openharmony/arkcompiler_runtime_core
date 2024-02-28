/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

/*---
desc: Hiding method of superinterface.
assert: If a class C declares or inherits a static method m, then m is said to hide any method m', where the signature of m is a subsignature  of the signature of m', in the superclasses and superinterfaces of C that would otherwise be accessible  to code in C.
---*/

interface I {
 static m(): int {
    return 1;
  }
}

class C implements I {
  public static m(): int {
    return 5;
  }
}

function main(): void {
  assert C.m() == 5;
}
