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
desc: Elements of Tuple value accessed via unsafeGet method.
---*/

function log_1(x: Object): string {
  if (x instanceof Tuple) {
    return "" + x.unsafeGet(1);
  }
  return "not a tuple";
}

function main(): int {
  let suite = new arktest.ArkTestsuite("Type Tuple unsafeGet");

  suite.addTest("unsafeGet returns correct element", () => {
    let a: [string, string] = ["aa", "bb"];
    let t: Tuple = a;
    arktest.assertEQ(t.unsafeGet(0), "aa");
    arktest.assertEQ(t.unsafeGet(1), "bb");
  });

  suite.addTest("unsafeGet with numeric tuple", () => {
    let a: [number, number] = [10, 20];
    let t: Tuple = a;
    arktest.assertEQ(t.unsafeGet(0), 10);
    arktest.assertEQ(t.unsafeGet(1), 20);
  });

  suite.addTest("unsafeGet returns Any type", () => {
    let a: [string, number] = ["hello", 99];
    let t: Tuple = a;
    let val: Any = t.unsafeGet(0);
    arktest.assertEQ(val, "hello");
  });

  suite.addTest("unsafeGet via Object parameter", () => {
    let a: [string, string] = ["aa", "bb"];
    arktest.assertEQ(log_1(a), "bb");
  });

  suite.addTest("unsafeGet with boolean element", () => {
    let a: [boolean, string] = [true, "yes"];
    let t: Tuple = a;
    arktest.assertEQ(t.unsafeGet(0), true);
    arktest.assertEQ(t.unsafeGet(1), "yes");
  });

  return suite.run();
}
