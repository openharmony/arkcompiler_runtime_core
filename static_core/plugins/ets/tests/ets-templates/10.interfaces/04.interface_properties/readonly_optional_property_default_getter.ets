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
desc: Readonly optional property provides default getter that returns undefined
params: "TEST-UNIT-10-0005; GAP-10-0005; CI-10-04-002-01"
assertion: "Default getter of a readonly optional property returns undefined"
test_suffix: "01"
---*/

interface WithReadonlyOptional {
  readonly tag?: number
}

class Holder implements WithReadonlyOptional {}

function main(): void {
  let holder: WithReadonlyOptional = new Holder()
  arktest.assertEQ(holder.tag, undefined,
    'default getter of a readonly optional property returns undefined')
}
