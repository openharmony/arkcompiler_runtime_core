/*
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

function GetNull(): null {
    return null;
}

function GetUndefined(): undefined {
    return undefined;
}

function GetObject(): Object {
    return new String("Hello World!");
}

class Pack {
    boolean_value: boolean = true;
    string_value: string = "";
    byte_value: byte = 0;
}

function newPackObject(): Pack {
    return new Pack();
}

class A {
    public static new_A(): A {
        return new A();
    }

    concat(s0: String, s1: String): String {
        return s0 + s1;
    }
}

class B extends A {
    public static new_B(): B {
        return new B();
    }

    concat(s0: String, s1: String): String {
        return s0 + "_" + s1;
    }
}
