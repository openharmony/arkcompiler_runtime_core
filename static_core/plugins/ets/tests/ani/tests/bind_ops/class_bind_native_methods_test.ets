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

class RegisteringNativeMethodsTest {
    public static native foo(): int;
    public static native long_foo(): long;

    public bar(): int {
        return 21;
    }
}

namespace test002A {
    namespace test002B {
        class TestA002 {
            constructor() {
            }
            public native foo(): int;
            public native long_foo(): long;
        }
    }
}

class TestA003 {
    constructor() {
    }
};

class TestB003 extends TestA003 {
    constructor() {
    }
    public native foo(): int;
    public native long_foo(): long;
};

final class TestA004 {
    constructor() {
    }
    public native foo(): int;
    public native long_foo(): long;
};

class TestA005 {
    constructor() {
    }

    foo(): int {
        return 50;
    }
};

class TestA006 {
    public native foo(x: int, y: int): int;
    public native foo(x: int, y: int, z: int): int;
}

interface Iface {
}

class Impl implements Iface {
}

class A {
    public native method_A(): int;
    public native method1(): Iface;
    public native method2(arg: Impl): void;
}

class B extends A {
    public native final method_B(): int;
    public override native method1(): Impl;
    public override native method2(arg: Iface): void;
}

class C extends B {
}