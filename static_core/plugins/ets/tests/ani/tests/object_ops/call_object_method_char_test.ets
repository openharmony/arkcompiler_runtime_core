/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

    public charMethod(a0: char, a1: char): char
    {
        return a0 > a1 ? a0 : a1;
    }

    public charMethodVoidParam(): char
    {
        return 'A';
    }

    public charMethodMultipleParam(a0: byte, a1: boolean, a2: float, a3: float, a4: byte): char
    {
        if (a1) {
            return 'A';
        } else if (a2 > a3) {
            return 'B';
        } else {
            return 'C';
        }
    }

    public nestedMethod(data1: char, data2: int): char
    {
        let method = (value1: char, value2: int): char => {
            return (value1 + value2) as char;
        }

        return method(data1, data2);
    }

    public recursionMethod(a0: char): char
    {
        if (a0 > 100) {
            return a0 as char;
        } else {
            return this.recursionMethod((a0 + 5) as char);
        }
    }
}

final class B extends A {

    public subCharMethodVoidParam(): char
    {
        return 'W';
    }
}

abstract class C {
    public abstract func(a: char, b: byte): char;
}

class D extends C {
    public override func(a: char, b: byte): char {
        return (a + b) as char;
    }
}

class E extends D {
    public override func(a: char, b: byte): char {
        return (a - b) as char;
    }
}
