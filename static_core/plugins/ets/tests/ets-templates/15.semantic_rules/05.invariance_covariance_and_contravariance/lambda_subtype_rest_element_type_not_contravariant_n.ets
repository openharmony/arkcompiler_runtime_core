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
desc: Lambda subtype rest element type not contravariant is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0318: Type '((p1: LambdaSubFail013Derived, ...p2:
        Array<LambdaSubFail013Derived>) => LambdaSubFail013Base)' cannot be assigned to type '((p1:
        LambdaSubFail013Base, ...p2: Array<LambdaSubFail013Base>) => LambdaSubFail013Base)'
---*/

'use static'

class LambdaSubFail013Base {}
class LambdaSubFail013Derived extends LambdaSubFail013Base {}

let source: (p: LambdaSubFail013Derived, ...rest: LambdaSubFail013Derived[]) => LambdaSubFail013Base =
  (p: LambdaSubFail013Derived, ...rest: LambdaSubFail013Derived[]): LambdaSubFail013Base => p

let target: (p: LambdaSubFail013Base, ...rest: LambdaSubFail013Base[]) => LambdaSubFail013Base = source
