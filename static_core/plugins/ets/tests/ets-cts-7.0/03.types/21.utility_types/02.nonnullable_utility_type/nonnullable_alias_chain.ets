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
desc: NonNullable in type alias chain - idempotency through aliases
---*/

type A = "hello"|"world"|null|undefined
type B = NonNullable<A>
type C = NonNullable<B>

function main() {
    let bnn: B = "hello"
    let cnn: C = "world"

    // check type via assignment
    let b: "hello"|"world" = bnn
    let c: "hello"|"world" = cnn
}
