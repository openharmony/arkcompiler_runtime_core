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
desc: "TEST-UNIT-11-0002 GAP-11-0002 GRC-11-0002 CI-11-02-001-01: Enum with explicit base type validates member types"
test_suffix: "01"
tags: [compile-only]
---*/
enum IntEnum: int {
  A = 1,
  B = 2,
  C = 3
}

enum StringEnum: string {
  A = "first",
  B = "second"
}

function main(): void {
  let intMember: IntEnum = IntEnum.A
  let stringMember: StringEnum = StringEnum.A
  console.log("Explicit base type validation passed")
}
