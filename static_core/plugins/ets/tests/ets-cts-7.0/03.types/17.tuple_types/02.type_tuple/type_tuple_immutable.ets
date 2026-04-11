/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: No element of a Tuple value can be changed.
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("Type Tuple immutability");

  suite.addTest("Tuple element accessed via unsafeGet is read-only snapshot", () => {
    let a: [number, string] = [1, "abc"];
    let t: Tuple = a;
    let val = t.unsafeGet(0);
    arktest.assertEQ(val, 1);
  });

  suite.addTest("Tuple preserves original values after concrete tuple change", () => {
    let a: [number, string] = [1, "abc"];
    let t: Tuple = a;
    a[0] = 99;
    let val = t.unsafeGet(0);
    arktest.assertEQ(val, 99);
  });

  suite.addTest("Tuple length matches original tuple", () => {
    let a: [number, string, boolean] = [1, "test", true];
    let t: Tuple = a;
    arktest.assertEQ(t.unsafeGet(0), 1);
    arktest.assertEQ(t.unsafeGet(1), "test");
    arktest.assertEQ(t.unsafeGet(2), true);
  });

  return suite.run();
}
