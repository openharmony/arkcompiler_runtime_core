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

    public double_method(a0: double, a1: double): double
    {
        return a0 + a1;
    }

    public doubleMethod(a0: double, a1: double): double
    {
        return a0 + a1;
    }

    public doubleMethodVoidParam(): double
    {
        return 5.0 as double;
    }

    public doubleMethodMultipleParam(a0: short, a1: boolean, a2:double, a3:short, a4:double): double
    {
        if (a1) {
            return (a2 + a4) as double;
        } else if (a0 > a3) {
            return (a2 - a4) as double;
        } else {
            return a2;
        }
    }

    public nestedMethod(data1: double, data2: double): double
    {
        let method = (value1: double, value2: double): double => {
            return (value1 + value2) as double;
        }

        return method(data1, data2);
    }

    public recursionMethod(a0: double): double
    {
        if (a0 > 100) {
            return a0;
        } else {
            return this.recursionMethod((a0 + 10.0) as double);
        }
    }
}

final class B extends A {

    public subDoubleMethodVoidParam(): double
    {
        return 8.0 as double;
    }
}

abstract class C {
    public abstract func(a: double, b: double): double;
}

class D extends C {
    public override func(a: double, b: double): double {
        return (a + b) as double;
    }
}

class E extends D {
    public override func(a: double, b: double): double {
        return (a - b) as double;
    }
}
