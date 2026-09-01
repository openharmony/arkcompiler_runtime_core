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
desc: "TEST-UNIT-11-0012 GAP-11-0012 GRC-11-0012 CI-11-00-003-02: Enum members can have same value"
---*/
enum SameValueEnum {
  A = 1,
  B = 1,
  C = 2
}

function main(): void {
  arktest.assertEQ(SameValueEnum.A.valueOf() as int, 1)
  arktest.assertEQ(SameValueEnum.B.valueOf() as int, 1)
  arktest.assertEQ(SameValueEnum.C.valueOf() as int, 2)

  // fromValue returns last member with that value
  let member = SameValueEnum.fromValue(1)
  arktest.assertEQ(member.getName(), 'B')
}
