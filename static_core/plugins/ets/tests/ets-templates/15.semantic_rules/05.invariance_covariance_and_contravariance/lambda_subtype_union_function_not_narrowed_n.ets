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
desc: Lambda subtype union function not narrowed is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0318: Type '(((p1: LambdaSubFail014Base) =>
        LambdaSubFail014Derived))|(((p1: LambdaSubFail014Derived) => LambdaSubFail014Base))' cannot
        be assigned to type '((p1: LambdaSubFail014Base) => LambdaSubFail014Derived)'
---*/

'use static'

class LambdaSubFail014Base {}
class LambdaSubFail014Derived extends LambdaSubFail014Base {}

type LambdaSubFail014Narrow = (p: LambdaSubFail014Base) => LambdaSubFail014Derived
type LambdaSubFail014Wide = (p: LambdaSubFail014Derived) => LambdaSubFail014Base
type LambdaSubFail014Union = LambdaSubFail014Narrow | LambdaSubFail014Wide

function useUnion(f: LambdaSubFail014Union): void {
  let onlyNarrow: LambdaSubFail014Narrow = f
}
