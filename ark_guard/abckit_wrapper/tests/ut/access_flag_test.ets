/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
abstract class AbstractClass {
}

final class FinalClass {
}

class ClassA {
    public static method0() {
    }

    public method1() {
    }

    protected method2() {
    }

    private method3() {
    }
}

abstract class ClassB {
    public final method0() {
    }

    public abstract method1();

    public async method2() {
    }

    public native method3(): void;
}

class ClassC {
    public static field0: int;
    public field1: int;
    protected field2: int;
    private field3: int;
}