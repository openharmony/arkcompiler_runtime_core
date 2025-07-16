/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Use type alias before declaration in the same scope
tags: []
---*/

const our: TOuterStringAlias = 'Outer String Alias'

type TOuterStringAlias = String


function body() {

    let x: TStringAlias = 'Hello Alias'
    type TStringAlias = String

    const y: TStringAlias = x
    return x
}

function main() {
    let oa: TOuterStringAlias = our
    let ba = body()
}
