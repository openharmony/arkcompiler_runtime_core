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
desc: "TEST-UNIT-11-0003 GAP-11-0003 GRC-11-0003 CI-11-02-003-01: Numeric enum supports arithmetic operators"
test_suffix: "01"
---*/
enum NumericEnum: int {
  A = 10,
  B = 20
}

function main(): void {
  let a: NumericEnum = NumericEnum.A
  let b: NumericEnum = NumericEnum.B
  let result: int = a.valueOf() + b.valueOf()
  arktest.assertEQ(result, 30)
}
