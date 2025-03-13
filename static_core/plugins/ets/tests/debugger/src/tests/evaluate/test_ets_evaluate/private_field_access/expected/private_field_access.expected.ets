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

package evalPatchFunction;

class A {
    public a: char;
    public b: double;

    constructor(a: char, b: double)
    {
        this.a = a;
        this.b = b;
    }

    public foo(val: int): int
    {
        return 100 * val;
    }

    public goo(val: double): double
    {
        return 200 * val;
    }
};

class evalPatchFunction {
    private static evalPatchFunction() {
        let a_obj = new A(c'c', 123.321);

        assertEQ(a_obj.a, c'c');
        assertEQ(a_obj.b, 123.321);

        assertEQ(a_obj.foo(42), 100 * 42);
        assertEQ(a_obj.goo(42.42), 200 * 42.42);
    }
}
