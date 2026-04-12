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
desc: unsafeGet causes runtime error when index is out of bounds.
tags: [no-warmup]
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("Type Tuple unsafeGet error");

  suite.addTest("unsafeGet with index equal to length throws", () => {
    let a: [string] = ["only"];
    let t: Tuple = a;
    arktest.expectError(() => {
      t.unsafeGet(1);
    });
  });

  suite.addTest("unsafeGet with negative index throws", () => {
    let a: [number, string] = [1, "two"];
    let t: Tuple = a;
    arktest.expectError(() => {
      t.unsafeGet(-1);
    });
  });

  suite.addTest("unsafeGet with large index throws", () => {
    let a: [number, number] = [5, 6];
    let t: Tuple = a;
    arktest.expectError(() => {
      t.unsafeGet(100);
    });
  });

  return suite.run();
}
