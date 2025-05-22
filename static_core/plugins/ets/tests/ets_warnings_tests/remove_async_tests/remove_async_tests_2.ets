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

let num_of_cycles = 1

class A {
    async foo() : Promise<String>
    {
        return "5"
    }
}

async function foo_1() : Promise<String> {
    return "5"
}

function bench_body() : void {
    let a = new A
    for(let i = 0; i < num_of_cycles; i++)
    {
        let promise = a.foo()
        promise = foo_1()
    }
}

function main() {
    bench_body()
}
