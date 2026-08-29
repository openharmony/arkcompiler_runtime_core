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
desc: "TEST-UNIT-09-0009 GAP-09-0009 GRC-09-0009 CI-09-03-002-01: Class implements required interface properties"
tags: [compile-only]
---*/

interface HasId {
    id: number
    getName(): string
}

class User implements HasId {
    public id: number
    private name: string

    constructor(id: number, name: string) {
        this.id = id
        this.name = name
    }

    getName(): string {
        return this.name
    }
}

function main(): void {
    let user = new User(1, 'Alice')
    console.log(user.id, user.getName())
}
