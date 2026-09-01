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
desc: "TEST-UNIT-11-0013 GAP-11-0013 GRC-11-0013 CI-11-00-008-01: Enum value implicitly converts to numeric type"
---*/
enum Status { OK = 200, NotFound = 404 }

function main(): void {
  let code: Status = Status.OK
  let num: number = code  // Implicit conversion
  arktest.assertEQ(num, 200)

  let result: int = Status.NotFound + 1
  arktest.assertEQ(result, 405)
}
