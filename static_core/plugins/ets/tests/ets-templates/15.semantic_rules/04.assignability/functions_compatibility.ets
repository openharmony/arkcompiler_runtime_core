/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
desc: Compatible functions have same number of arguments. Arguments and return values should follow the variance rule.
---*/

function foo(i: int) {
    return 1;
}

function foo2(i: int): Int {
    return 2;
}

function foo3(i: int|string): Int {
    return 3;
}

function foo4(i: int|string, j?: int): Int {
    return 4;
}


function main() {
    let x: (i: int) => int
    x = foo
    arktest.assertEQ(1, x(1));
    x = foo2
    arktest.assertEQ(2, x(1));
    x = foo3
    arktest.assertEQ(3, x('1'));
    x = foo4
    arktest.assertEQ(4, x('1'));
}
