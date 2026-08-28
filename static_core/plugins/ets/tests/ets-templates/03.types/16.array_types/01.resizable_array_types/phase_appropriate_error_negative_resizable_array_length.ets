/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: A negative resizable-array length reports the phase-appropriate error.
test_unit: TEST-UNIT-03-0062
assertion: GAP-03-0094 / GRC-03-0120 / CI-03-16-01-012-01
---*/
function main(): void {
  let values: number[] = [1, 2]
  let checkRangeError = (e: Error): boolean => { return e instanceof RangeError }
  arktest.expectThrow(() => { values.length = -1 }, checkRangeError)
}
