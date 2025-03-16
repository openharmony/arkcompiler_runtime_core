/**
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
class A {
    foo1() : long {
        let a: long = 5;
        return a;
    }
    foo2() : number {
        let a: number = 7.1;
        return a;
    }
    foo3() : long {
        let a: long = -5;
        return a;
    }
}

function main() {
    let a = new A()
    console.log(a.foo1())
    console.log(a.foo2())
    console.log(a.foo3())
}
