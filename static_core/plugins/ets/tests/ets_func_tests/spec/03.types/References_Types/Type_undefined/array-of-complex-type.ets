/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
desc: 3.8.9 The only value of type undefined is represented by the keyword undefined
name: array-of-complex-type.ets
tags:

---*/

class ComplexClass {
    n : number | undefined | null = null;
    s : string | undefined | null = null;
}

type T_ = ComplexClass | undefined | null;
type T = Array<ComplexClass> | Array<ComplexClass | undefined> | Array<ComplexClass | null> | Array<T_> | T_;

function check(b : boolean): int {
    return b ? 0 : 1;
}

function callMe<T>(p : T): T {
    if (p === undefined)
        return null;

    if (p === null)
        return undefined;

    if (p instanceof ComplexClass)
        return { n: p.s?.length, s: p.n?.toString() };

    if (p instanceof Array) {
        let tmp = p[0]
        if (tmp === null)
            return null;
        if (tmp === undefined)
            return undefined;
        if (tmp instanceof ComplexClass)
            return [null, undefined, new ComplexClass(), {n: null, s: undefined}]
    }

    return new Array<ComplexClass | undefined>();
}

function main(): int {
    let result: int = 0;
    let arr08 = new Array<ComplexClass | undefined | null>();

    arr08.push(null);
        result += check(callMe(arr08) === arr08[0] && arr08[0] !== undefined);
    arr08.pop();

    arr08.push(undefined);
        result += check(callMe(arr08) === arr08[0] && arr08[0] !== null);
    arr08.pop();

    arr08.push(new ComplexClass());
        result += check((callMe(arr08) as Array<T_>).length === 4);
        result += check((callMe(arr08) as Array<T_>)[3]!.n === null);
        result += check((callMe(arr08) as Array<T_>)[3]!.s === undefined);
        result += check((callMe(arr08) as Array<T_>)[3]!.n === (callMe(arr08) as Array<ComplexClass | undefined | null>)[0]);
        result += check((callMe(arr08) as Array<T_>)[3]!.s === (callMe(arr08) as Array<ComplexClass | undefined | null>)[1]);
        result += check((callMe(arr08) as Array<T_>)[2] instanceof C11)
    arr08.pop();

    return result;
}
