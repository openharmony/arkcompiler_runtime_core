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
desc: >-
    Cases where a method and a function have the same name
---*/

// base cases
class A {
    foo (): string { return "the instance method"; }
}
function foo(this: A): string { return "the function"; }

// both with generic
class B {
    bar<T> (): string { return "the instance method"; }
}
function bar<T>(this: B): string { return "the function"; }

// with generic and not 
class C {
    func(): C { return this; }
}
function func<T extends Object>(this: T): T { return this }

function main(): int {
    let testsuite = new arktest.ArkTestsuite("Method and function with same name");
    
    testsuite.addTest("The instance method has a priority over the implicitly called function with receiver", () => {
        arktest.assertEQ(
            (new A).foo(), "the instance method"
        );
    });

    testsuite.addTest("The function with receiver still can be called explicitly", () => {
        arktest.assertEQ(
            foo(new A), "the function"
        );
    });

    testsuite.addTest("The method and the function with generics", () => {
        arktest.assertEQ(bar<boolean>(new B), "the function");
        arktest.assertEQ((new B).bar<int>(), "the instance method");
    });

    testsuite.addTest("The function with generics", () => {
        arktest.assertTrue(func(new C) instanceof C);
        arktest.assertTrue((new C).func<C>() instanceof C);
    });

    return testsuite.run();
}
