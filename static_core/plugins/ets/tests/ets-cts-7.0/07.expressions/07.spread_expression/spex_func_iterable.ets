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
desc: A spread expression can be used in function calls (built-in iterable sources).
---*/

function fIterRecArr(...p: Object[]): number { return p.length }
function fIterStr(...p: string[]): string { return String.join(p, '-') }
function fIterRecTup(...p: Array<[int, boolean]>): string {
    let res = ''
    for (let e of p) {
        res += e[0] + ':' + e[1] + ','
    }
    return res
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Spread of built-in iterables in function calls")

    suite.addTest("spread of record directly", () => {
        let r: Record<int, string> = {1: '1', 3: '3'}
        arktest.assertEQ(fIterRecArr(...r), 2)
    })

    suite.addTest("spread of iterable of strings", () => {
        let a = 'iterable'
        arktest.assertEQ(fIterStr(...a), 'i-t-e-r-a-b-l-e')
    })

    suite.addTest("spread of iterable of tuples", () => {
        let a: Record<int, boolean> = {
            1: true,
            2: false,
        }
        arktest.assertEQ(fIterRecTup(...a), '1:true,2:false,')
    })

    return suite.run()
}
