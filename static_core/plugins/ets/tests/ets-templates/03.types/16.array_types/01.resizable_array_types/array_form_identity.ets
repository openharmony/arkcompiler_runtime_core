/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: T[] and Array<T> specify identical types and are assignable to each other.
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("Array Form Assignment");

  suite.addTest("Assign Array<T> to T[]", () => {
    let form1: Array<String> = []
    let form2: String[] = []
    form2 = form1
    arktest.assertEQ(form1.length, 0)
    arktest.assertEQ(form2.length, 0)
  });

  suite.addTest("Assign T[] to Array<T>", () => {
    let form1: Array<String> = []
    let form2: String[] = []
    form1 = form2
    arktest.assertEQ(form1.length, 0)
    arktest.assertEQ(form2.length, 0)
  });

  return suite.run()
}
