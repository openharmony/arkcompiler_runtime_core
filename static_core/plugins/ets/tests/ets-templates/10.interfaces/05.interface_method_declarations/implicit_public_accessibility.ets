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
desc: Interface method has implicit public access level
params: "TEST-UNIT-10-0007; GAP-10-0007; CI-10-05-001-01"
assertion: "Interface method with implicit public access is callable from any context"
test_suffix: "01"
---*/

interface PublicByDefault {
  describe(): string
}

class Implementor implements PublicByDefault {
  describe(): string {
    return 'visible'
  }
}

function callThroughInterface(instance: PublicByDefault): string {
  return instance.describe()
}

function main(): void {
  arktest.assertEQ(callThroughInterface(new Implementor()), 'visible',
    'interface method with implicit public access is callable from any context')
}
