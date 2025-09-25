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

function foo() : never {
    throw new Error('foo() never returns')
}

class AnyTypeTest{
    private a: Any;
}

class NeverTypeTest{
    private a: never = foo();
}

function foo1(a: Any): void {}

function foo2(a: never): void {}

function foo3(a: Any, b: never): void {}

function foo4(a: Any[]): void {}

function foo5(a: Any, b: Any[]): void {}

function foo6(a: never, b: Any[]): void {}

function foo7(a: Any): Any {
    return a;
}

function foo8(a: never): never {
    return a;
}

function foo9(a: Any, b: never): void {}

function foo10(a: Any[]): Any[] {
    return a;
}

function foo12(a: never): Any {
    return (a as Any);
}

class AnyTypeTest2{
    foo1(x: Any): void {}
    foo2(x: Any): Any {return x}
    foo3(x: never): void {}
    foo4(x: never): never {return x}
    foo5(x: Any[]): void {}
    foo6(x: Any[]): Any[] {return x}
    foo7(x: int, y: Any[]): void {}
    foo8(x: int, y: never): void {}
}
