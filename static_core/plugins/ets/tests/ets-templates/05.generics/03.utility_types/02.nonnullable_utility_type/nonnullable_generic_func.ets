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
desc: NonNullable in generic function signature - return type excludes null
---*/

function skipTypeInference<T>(x: T): NonNullable<T> {
    return x as NonNullable<T>
}

function main() {

    let sn: NonNullable<string|null>  = skipTypeInference<string|null>("str")
    let s: string = sn

    let bn: NonNullable<boolean|undefined>  = skipTypeInference<boolean|undefined>(true)
    let b: boolean = bn

    let nnu: NonNullable<number|null|undefined>  = skipTypeInference<number|null|undefined>(42)
    let n: number = nnu
}