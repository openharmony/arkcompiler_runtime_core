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
desc: NonNullable with tuple types - resolves to the tuple type
---*/

type T = [string, number]

function main() {
    let t: NonNullable<T|null> = ["hello", 42]
    let tt: T = t

    let t2: NonNullable<[boolean, string]|null|undefined> = [true, "world"]
    let tt2: [boolean, string] = t2
}
