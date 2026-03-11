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
  Constant enumeration members must be initialized by compile-time expressions.
  A function call is not a constant expression.
tags: [compile-only, negative]
expected_out: >-
  Semantic error ESE0342: Enumeration members can be initialized only by
  compile-time expressions and initializers must be of the same type.
---*/

function getVal(): int {
    return 5
}

const enum BadEnum {
    A = getVal()
}
