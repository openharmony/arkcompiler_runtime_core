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

namespace anyns {
    export let aBool: boolean = false;
    export let aDouble: double = 3.0;
    export let sum: double;
    class A {
        static val1: double = 1.5;
        static val2: double = 2.5;
        public static add(): double {
            sum = A.val1 + A.val2;
            return sum;
        }
    }

    namespace second {
        export let aValue: double = 5.0;
    }
}

namespace anyns {
    export let testValue: double = 3.14;
}