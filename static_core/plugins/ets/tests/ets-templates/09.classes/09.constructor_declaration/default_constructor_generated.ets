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
  TEST-UNIT-09-0016 GAP-09-0016 GRC-09-0016 CI-09-09-004-01: Default
  constructor is generated when no constructor declared
tags: [compile-only]
---*/

class SimpleClass {
    public value: number = 0
}

function main(): void {
    let obj = new SimpleClass()  // Uses default constructor
    console.log(obj.value)
}
