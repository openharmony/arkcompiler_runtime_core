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
  Bigint with literal negative exponent causes a compile-time error.
  If the second operand of type bigint is negative,
  then a compile-time error occurs when detected at compile time.
tags: [compile-only, negative]
expected_out: 'Semantic error ESE655064: Exponentiation expressions cannot have a negative bigint exponent.'
---*/

function main(): void {
    let x = 2n ** -1n
}
