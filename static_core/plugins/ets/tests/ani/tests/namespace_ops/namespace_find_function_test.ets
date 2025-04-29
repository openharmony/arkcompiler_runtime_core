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

namespace fnns {
    function getInitialIntValue(): int {
        return 0;
    }
    function getInitialStringValue(): String {
        return "";
    }
    function overloaded(i: int): int {
        return 0
    }
    function overloaded(): int {
        return 1
    }
    function getInitialDoubleValue(): double {
        return 1.00;
    }

    abstract class B {
        abstract getInitialDoubleValue(): double;
    }

    class A extends B {
        constructor(){};
        static B(): double {
            return 1.00;
        };
        getInitialDoubleValue(): double {
            return 1.00;
        }

        getInitialIntValue(): int {
            return 0;
        }
        getInitialIntValue(a: int, b: int): int {
            return a + b;
        }
    }

    async function C(a: int): Promise<int>{
        return a;
    };

}

namespace testA {
    namespace A {
        function B (): int {
            return 1;
        }
    }
}