/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Bounds test
---*/

class A {}
class BA extends A {}
class CBA extends BA {}
class DA extends A {}

function foo(p: Object): int {
  return 1;
}

function foo(p: A): int {
  return 2;
}

function foo(p: BA): int {
  return 3;
}

function main(): void {
  let a: CBA = new CBA();
  let b: DA = new DA();
  let c = true ? a : b;
  assertTrue(foo(c) == 2);

  let d: A = new A();
  let e = true ? a : d;
  assertTrue(foo(e) == 2);

  let f: BA = new BA();
  let g = true ? a : f;
  assertTrue(foo(g) == 3);
}
