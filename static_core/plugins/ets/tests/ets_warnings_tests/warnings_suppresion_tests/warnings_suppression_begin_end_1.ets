/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

// ETSNOLINT-BEGIN






class A
{
    async foo() : Promise<String>
    {
        return "5";
    }
}







function bench_body() : void
{
    let a = new A;
    for(let i = 0; i < 10; i++)
    {
        let promise = a.foo();

    }
}


class AA {
    s: String = "Hello, World!";
    foo(): void {
        console.println(this.s);
    }
}


function main()
{
    bench_body()
    let a : Number[] = new Number[10]
    let x : Number = 5.0
    for (let x : Number of a) {
    }

    let foo: (i : int) => int
    foo = (i : int): int => {return i+1}

    let TOTAL_NUMBER = 10

    let tmp = 0
    for (let i = 0; i < TOTAL_NUMBER; i++) {
        tmp = foo(tmp)
    }
    let ref: Int = new Int(3);
    for (let aa : int = 1; aa < 10; aa++) {
        if (ref != null) {
            ref = ref + 1;
        }

        let new_ref: AA = new AA();
        if (new_ref != null) {
            ref = ref + 3;
        }
    }
}
// ETSNOLINT-END
