/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

package evalPatchFunction;

import { E } from "../interfaces_2.base"

class evalPatchFunction {
    private static evalPatchFunction() {
        let e = new E();
        assertEQ(e.a_foo(42) ,42);
        assertEQ(e.b_foo(12345.54321), 12345.54321);
        assertEQ(e.c_foo(c'a'), c'a');
        assertEQ(e.d_foo(123.321 as float), 123.321 as float);
    }
}
