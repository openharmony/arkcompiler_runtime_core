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

class InterfaceNativeTest {
    static {
        loadLibrary("etsnapi_lib");
    }

    public static C: char = c'\u03a9';

    public static FA: float[] = [0 as float , 1e2 as float, (Float.NEGATIVE_INFINITY)];

    public native static callMethod(): void;

    public native static testChar(c: char): void;

    public native static testNewObject(): int;

    public native static testStrUTFLen(str1: String, str2: String): int;

    public native static testNewStrUTF(str: String): String;

    public native static testGetStrUTF(str: String): int;

    public native static testFloat(f1: float, f2: float, f3: float): int;

    public static staticLongMethod(a: int): long {
        console.println(a);
        return a * (1 as long);
    }

    public static staticVoidMethod(c: char): void {
        if (c != InterfaceNativeTest.C) {
            throw new Exception("ERROR: error arg value from ETSNAPI");
        }
    }

    public static staticFloatMethod(i: int): float {
        return InterfaceNativeTest.FA[i];
    }

    public mCode: int;

    public constructor(code: int) {
        this.mCode = 1 / code;
    }

    class Inner {
    }

    public run(): void {
        InterfaceNativeTest.callMethod();
        InterfaceNativeTest.testChar(InterfaceNativeTest.C);

        if (InterfaceNativeTest.testFloat(InterfaceNativeTest.FA[0], InterfaceNativeTest.FA[1], InterfaceNativeTest.FA[2]) != 0) {
            throw new Exception("ERROR: error return value from ETSNAPI");
        }

        if (InterfaceNativeTest.testNewObject() != 0) {
            throw new Exception("ERROR: error return value from ETSNAPI");
        }

        let str1: String = "";
        let str2: String = "Z\u0000Y\u0080X\u06efW\u07ffV\u0800U\u5555T\uFFFFS";
        if (InterfaceNativeTest.testStrUTFLen(str1, str2) != 25) {
            throw new Exception("ERROR: error return value from ETSNAPI");
        }

        if (!(str2 == InterfaceNativeTest.testNewStrUTF(str2))) {
            throw new Exception("ERROR: error return value from ETSNAPI");
        }

        let hellostr: String = "HELLO WORLD!";
        if (InterfaceNativeTest.testGetStrUTF(hellostr) != 0) {
            throw new Exception("ERROR: error return value from ETSNAPI");
        }
    }
}

function main(): void {
    let a: InterfaceNativeTest = new InterfaceNativeTest(5);
    a.run();
}
