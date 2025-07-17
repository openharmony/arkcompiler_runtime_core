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
    param: string = "123";
    foo(a: string): void {
      console.log(a);
    }
}

class MyClass {
    prop: string;
    constructor() {
        this.prop = "class prop";
    }
}

function InitFuncName(a: string, inst: MyClass) {
  console.log(a)
  console.log(inst.prop)
}

function add1(a: int, b: int): int {
    return a;
}
function add2(a: int, b: int): number {
    return a;
}
function add3(a: int, b: int): string {
    return "123";
}
function main() {
    let classA = new A();
    InitFuncName("123", new MyClass());
    console.log(add1(1, 2));
    console.log(add3(1, 2));
    classA.foo("foo");
}