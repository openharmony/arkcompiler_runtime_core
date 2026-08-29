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
  TEST-UNIT-11-0009 GAP-11-0009 GRC-11-0009 CI-11-00-006-02: Enum
  initializer can reference members without qualification
---*/
enum RefEnum {
  A = 10,
  B = A,      // OK - can omit qualification in initializer
  C = B + 5
}

function main(): void {
  arktest.assertEQ(RefEnum.A.valueOf() as int, 10)
  arktest.assertEQ(RefEnum.B.valueOf() as int, 10)
  arktest.assertEQ(RefEnum.C.valueOf() as int, 15)
}
