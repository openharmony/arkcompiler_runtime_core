/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Unary plus operator
---*/

function main(): void {
    arktest.assertEQ(+42, 42);
    arktest.assertEQ(+2 + +2, 4);
    arktest.assertEQ(+(+(+(+13))), 13);
    arktest.assertEQ(+1.5e+3, 1500.0);
    arktest.assertEQ(+0xff, 0xff);
    arktest.assertEQ(+0b10, 0b10);
    arktest.assertEQ(+0o70, 0o70);
    arktest.assertEQ(+0.5f, 0.5f);
    arktest.assertEQ(+42n, 42n);

    let b: byte = +127
    arktest.assertEQ(+b, b)
    arktest.assertEQ(+b, 127)

    let s: short = +32767
    arktest.assertEQ(+s, s)
    arktest.assertEQ(+s, 32767)

    let i: int = +-2147483648
    arktest.assertEQ(+i, i)
    arktest.assertEQ(+i, -2147483648)

    let l: long = +-9223372036854775808
    arktest.assertEQ(+l, l);
    arktest.assertEQ(+l, -9223372036854775808);

    let bi: bigint = +42n
    arktest.assertEQ(+bi, bi);
    arktest.assertEQ(+bi, 42n);
    arktest.assertEQ(+(new BigInt(13)), 13n);

    let n: number = -3.14;
    arktest.assertEQ(+n, n);
    arktest.assertEQ(+n, -3.14);
    arktest.assertEQ(n + +n, -6.28);

    let d: double = 3.14;
    arktest.assertEQ(+d, d);
    arktest.assertEQ(+d, 3.14);

    let f: float = 3.14f;
    arktest.assertEQ(+f, f);
    arktest.assertEQ(+f, 3.14f);

}
