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

import { boolean_foo, byte_foo, short_foo, int_foo, long_foo, double_foo, float_foo } from "../primitive_types.base";

class evalPatchFunction {
    private static evalPatchFunction() {
        assertEQ(boolean_foo(true), true);
        assertEQ(byte_foo(127 as byte), 127);
        assertEQ(short_foo(32767 as short), 32767);
        assertEQ(int_foo(50000 as int), 50000);
        assertEQ(long_foo(50000000 as long), 50000000);
        assertEQ(double_foo(12345.54321 as double), 12345.54321);
        assertEQ(float_foo(12345.5 as float), 12345.5 as float);
    }
}
