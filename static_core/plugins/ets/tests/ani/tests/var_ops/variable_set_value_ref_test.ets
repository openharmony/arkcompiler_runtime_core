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
    export let shortValue: short = 3;
    export let stringValue: String = "test";
    export let sum: String;
    class A {
        static val1: String = "hello";
        static val2: String = "world";
        public static add(): String {
            sum = A.val1 + A.val2;
            return sum;
        }
    }

    namespace second {
        export let aValue: String = "Chian";
    }
}

namespace anyns {
    export let testValue: String = "HongKong";
}