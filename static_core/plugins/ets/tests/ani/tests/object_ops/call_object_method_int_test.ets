/**
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

class A {
    public static newA()
    {
        return new A();
    }

    public intMethod(a0: int, a1: int): int
    {
        return a0 + a1;
    }

    public intMethodVoidParam(): int
    {
        return 5 as int;
    }

    public intMethodMultipleParam(a0: int, a1: boolean, a2:float, a3:short, a4:double): int
    {
        if (a1) {
            return a0 + a3;
        } else if (a4 > a2) {
            return a0;
        } else {
            return a0 - a3;
        }
    }

    public nestedMethod(data1: int, data2: int): int
    {
        let method = (value1: int, value2: int): int => {
            return value1 + value2;
        }

        return method(data1, data2);
    }

    public recursionMethod(a0: int): int
    {
        if (a0 > 100) {
            return a0;
        } else {
            return this.recursionMethod(a0 + 10);
        }
    }
}

final class B extends A {

    public subIntMethodVoidParam(): int
    {
        return 8 as int;
    }
}

abstract class C {
    public abstract func(a: int, b: int): int;
}

class D extends C {
    public override func(a: int, b: int): int {
        return (a + b) as int;
    }
}

class E extends D {
    public override func(a: int, b: int): int {
        return (a - b) as int;
    }
}
