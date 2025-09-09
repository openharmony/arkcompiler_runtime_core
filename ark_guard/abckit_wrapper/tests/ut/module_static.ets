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
function foo1(a: int, b: int) {
}

const m1 = 1;

interface Interface1 {
    iField1: int;

    iMethod1();
}

class ClassA implements Interface1{
    public static sField1 = 0;

    public field1 = '';
    public iField1 = 0;

    public static sMethod1(a: int){
    }

    public method1() {
    }

    public iMethod1() {
    }
}

enum Color1 {
    RED,
}

namespace Ns1 {
    namespace Ns2 {
    }

    function foo2(a: int, b: int) {
    }

    const m2 = 1;

    interface Interface2 {
        iField2: string;

        iMethod2();
    }

    class ClassB implements Interface2{
        public static sField2 = 0;

        public field2 = '';
        public iField2 = '';

        public static sMethod2(a: int){
        }

        public method2() {
        }

        public iMethod2() {
        }
    }

    enum Color2 {
        YELLOW,
    }
}