/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

class A {}

class B extends A{}

class C {
    public void_method(a: int, b: int): void
    {
    }

    public object_method(): A
    {
        let a : A = new A;
        return a;
    }

    public boolean_method(a: boolean, b: int): boolean
    {
        return false;
    }

    public byte_method(a: byte, b: int): byte
    {
        return 0;
    }

    public char_method(a: char, b: int): char
    {
        return 0;
    }

    public short_method(a: short, b: int): short
    {
        return 0;
    }

    public int_method(): int
    {
        return 0;
    }

    public long_method(a: long, b: int): long
    {
        return 0;
    }

    public float_method(a: float, b: int): float
    {
        return 0.0;
    }

    public double_method(a: double, b: int): double
    {
        return 0.0;
    }
}

final class D extends C {
    constructor(x: int) {
        this.member = 0;
    }

    public override void_method(a: int, b: int): void
    {
        this.member = a;
    }

    public override object_method(): A
    {
        this.member = 42;
        let a : A = new A;
        return a;
    }

    public override boolean_method(a: boolean, b: int): boolean
    {
        return true;
    }

    public override byte_method(a: byte, b: int): byte
    {
        return 1;
    }

    public override char_method(a: char, b: int): char
    {
        return 1;
    }

    public override short_method(a: short, b: int): short
    {
        return 1;
    }

    public override int_method(): int
    {
        return 1;
    }

    public override long_method(a: long, b: int): long
    {
        return 1;
    }

    public override float_method(a: float, b: int): float
    {
        return 1.0;
    }

    public override double_method(a: double, b: int): double
    {
        return 1.0;
    }

    public member : int;
}

class F {
    public static void_method(a: int): void
    {
    }

    public static object_method(): A
    {
        let a : A = new A;
        return a;
    }

    public static boolean_method(a: boolean): boolean
    {
        return false;
    }

    public static byte_method(a: byte): byte
    {
        return 0;
    }

    public static char_method(a: char): char
    {
        return 0;
    }

    public static short_method(a: short): short
    {
        return 0;
    }

    public static int_method(): int
    {
        return 0;
    }

    public static long_method(a: long): long
    {
        return 0;
    }

    public static float_method(a: float): float
    {
        return 0;
    }

    public static double_method(a: double): double
    {
        return 0;
    }
}
