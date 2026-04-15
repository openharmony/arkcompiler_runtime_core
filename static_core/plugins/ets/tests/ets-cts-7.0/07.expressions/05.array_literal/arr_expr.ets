/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: >-
  Initializer expressions are executed from left to right.
  The n’th expression specifies the value of the n-1’th element of the array.
---*/

let expected: int[] = [101, 203, -298, 304, 602,]
function multiplyBy100(cnt: int): int {
    return 100 * cnt;
}

function createRange(cnt: int): int[] {
    let res: int[] = new int[cnt](0)
    for (let i = 0; i < res.length; i++) {
        res[i] = i * 10
    }
    return res;
}

function main(): int {
    let suite = new arktest.ArkTestsuite("arr_expr")

    suite.addTest("side effects in int[] initializer expressions", () => {
        let cnt: int = 0
        let a: int[] = [multiplyBy100(++cnt) + cnt++, ++cnt + multiplyBy100(--cnt), cnt++ - multiplyBy100(cnt--), multiplyBy100(++cnt) + ++cnt, 2 * multiplyBy100(--cnt) + --cnt,]
        for (let i = 0; i < a.length; i++) {
            arktest.assertTrue( expected[i] == a[i] )
        }
    })

    suite.addTest("side effects in int[][] initializer expressions", () => {
        let cnt: int = 0
        let a: int[][] = [createRange(cnt++), createRange(cnt++), createRange(cnt++), createRange(cnt++), createRange(cnt++)]
        for (let i = 0; i < a.length; i++) {
            arktest.assertTrue( i == a[i].length )
            for (let j = 0; j < i; j++) {
                arktest.assertTrue( a[i][j] == j * 10 )
            }
        }
    })

    return suite.run()
}
