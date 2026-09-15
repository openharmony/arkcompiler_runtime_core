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
desc: Lambda subtype nested callback parameter type reversed is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0318: Type '((p1: ((p1: LambdaSubFail012Base) => LambdaSubFail012Derived))
        => Int)' cannot be assigned to type '((p1: ((p1: LambdaSubFail012Derived) =>
        LambdaSubFail012Base)) => Int)'
---*/

'use static'

class LambdaSubFail012Base {}
class LambdaSubFail012Derived extends LambdaSubFail012Base {}

type LambdaSubFail012NarrowCallback = (p: LambdaSubFail012Base) => LambdaSubFail012Derived
type LambdaSubFail012WideCallback = (p: LambdaSubFail012Derived) => LambdaSubFail012Base

let source: (cb: LambdaSubFail012NarrowCallback) => int =
  (cb: LambdaSubFail012NarrowCallback): int => 12

let target: (cb: LambdaSubFail012WideCallback) => int = source
