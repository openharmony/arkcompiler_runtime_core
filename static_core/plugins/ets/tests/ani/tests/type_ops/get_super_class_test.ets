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

class A {}

class B extends A {}

class C extends B {}

class Person {
    add_method(a0: int, a1: int): int {
        return a0 + a1;
    }
};

class Student extends Person {
    add_method(a0: int, a1: int): int
    {
        return a0 - a1;
    }
};

class Child extends Student {
    add_method(a0: int, a1: int): int
    {
        return a0 * a1;
    }
};

namespace test001 {
    class Parent {}
    class Child extends Parent {}
}

namespace test002 {
    namespace test003 {
        class Parent {}
        class Child extends Parent {}
    }
}