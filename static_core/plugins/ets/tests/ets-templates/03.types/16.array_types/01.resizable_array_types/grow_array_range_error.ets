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
desc: Increasing array length throws RangeError for both T[] and Array<T> forms.
---*/

function grow(seed: int): int {
  return 5 + seed
}

function main(): int {
  let suite = new arktest.ArkTestsuite("Grow Array RangeError");
  let checkRangeError = (e: Error): boolean => { return (e instanceof RangeError) }

  suite.addTest("T[] form with literal length throws RangeError", () => {
    let arr: String[] = ["Hello", "World", "Of", "ArkTS"]
    arktest.expectThrow(() => { arr.length = 5 }, checkRangeError)
  });

  suite.addTest("T[] form with computed length throws RangeError", () => {
    let arr: String[] = ["Hello", "World", "Of", "ArkTS"]
    arktest.expectThrow(() => { arr.length = grow(0) }, checkRangeError)
  });

  suite.addTest("Array<T> form with literal length throws RangeError", () => {
    let arr: Array<String> = ["Hello", "World", "Of", "ArkTS"]
    arktest.expectThrow(() => { arr.length = 5 }, checkRangeError)
  });

  suite.addTest("Array<T> form with computed length throws RangeError", () => {
    let arr: Array<String> = ["Hello", "World", "Of", "ArkTS"]
    arktest.expectThrow(() => { arr.length = grow(0) }, checkRangeError)
  });

  return suite.run()
}
