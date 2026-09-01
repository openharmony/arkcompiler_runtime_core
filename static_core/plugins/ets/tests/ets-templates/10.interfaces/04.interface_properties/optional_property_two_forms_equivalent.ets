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
desc: Optional property with ? and explicit | undefined are equivalent
params: "TEST-UNIT-10-0006; GAP-10-0006; CI-10-04-001-01"
assertion: "Both optional property forms have effective type T | undefined"
test_suffix: "01"
---*/

interface ShortForm {
  value?: number
}

interface ExplicitForm {
  value: number | undefined
}

class ShortHolder implements ShortForm {
  public value?: number
}

class ExplicitHolder implements ExplicitForm {
  public value: number | undefined
}

function main(): void {
  let shortHolder = new ShortHolder()
  let explicitHolder = new ExplicitHolder()
  arktest.assertEQ(shortHolder.value, undefined,
    'short form property defaults to undefined')
  arktest.assertEQ(explicitHolder.value, undefined,
    'explicit form property defaults to undefined')
  shortHolder.value = 42
  explicitHolder.value = 42
  arktest.assertEQ(shortHolder.value, explicitHolder.value,
    'both optional property forms have effective type T | undefined')
}
