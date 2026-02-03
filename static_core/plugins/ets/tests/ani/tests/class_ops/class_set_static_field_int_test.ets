/*
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
class TestSetInt {
    static int_value: int = 3;
    static string_value: string = "";
}

class TestSetIntA extends TestSetInt {
    static int_value: int = 3;
}

final class TestSetIntFinal extends TestSetInt {
    static int_value: int = 3;
}

class Parent {
    static parentStaticField: int = 0;
    static overridedStaticField: int = 0;
}
class Child extends Parent {
    static overridedStaticField: int = 0;
    static childStaticField: int = 0;
}
final class Grandchild extends Child {
    static grandchildStaticField: int = 0;
}
