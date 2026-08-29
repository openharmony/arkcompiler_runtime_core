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
desc: "TEST-UNIT-09-0010 GAP-09-0010 GRC-09-0010 CI-09-05-001-01: Private members cannot be accessed outside class"
tags: [compile-only, negative]
expected_out: |-
    Semantic error ESE0293: Property balance is not visible here.
---*/

class BankAccount {
    private balance: number

    constructor(initial: number) {
        this.balance = initial
    }

    getBalance(): number {
        return this.balance
    }
}

function main(): void {
    let account = new BankAccount(1000)
    console.log(account.balance)  // Should fail: balance is private
}
