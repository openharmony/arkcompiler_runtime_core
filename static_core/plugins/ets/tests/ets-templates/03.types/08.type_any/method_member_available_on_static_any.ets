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
# Traceability: TEST-UNIT-03-0033; GAP=GAP-03-0059; GRC=GRC-03-0080; CI=CI-03-08-003-01
desc: A method member access on a value with static type Any is rejected at compile time.
tags: [compile-only, negative]
---*/

function acceptAny(value: Any): Any {
    return value
}

function main(): void {
    let value: Any = acceptAny(1 as int)
    value.missingMethod()
}
