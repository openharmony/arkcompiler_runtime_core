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
desc: Push and assign to empty array for both T[] and Array<T> forms.
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("Put To Empty Array");

  suite.addTest("Push and assign to empty T[] form", () => {
    let arr: String[] = []
    arr.push("")
    arr[0] = "Hello"
    arktest.assertEQ(arr.length, 1)
    arktest.assertEQ(arr[0], "Hello")
  });

  suite.addTest("Push and assign to empty Array<T> form", () => {
    let arr: Array<String> = []
    arr.push("")
    arr[0] = "Hello"
    arktest.assertEQ(arr.length, 1)
    arktest.assertEQ(arr[0], "Hello")
  });

  return suite.run()
}
