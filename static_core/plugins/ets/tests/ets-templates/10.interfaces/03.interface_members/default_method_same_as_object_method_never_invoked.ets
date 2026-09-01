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
desc: Default method with same signature as Object method is never invoked
params: "TEST-UNIT-10-0003; GAP-10-0003; CI-10-03-002-01"
assertion: "Object.toString() must be called instead of the interface default method"
test_suffix: "01"
---*/

interface DefaultToString {
  toString(): string {
    return "interface default"
  }
}

class Carrier implements DefaultToString {}

function main(): void {
  let carrier = new Carrier()
  arktest.assertNE(carrier.toString(), "interface default",
    'Object.toString() must be called instead of the interface default method')
}
