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
desc: >-
  TEST-UNIT-03-0065 GAP-03-0097 GRC-03-0123 CI-03-16-01-016-02: assigning the existing length preserves
  the array
---*/
function main(): void {
  let values: number[] = [11, 22, 33]
  let oldLength = values.length
  values.length = oldLength
  arktest.assertEQ(values.length, oldLength)
  arktest.assertEQ(values[2], 33)
}
