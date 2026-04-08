/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

/*---
desc: >-
    Two libs imported, only lib1 changes (body modification).
    Lib2 stays unchanged. Test must pass with both old and new lib1.
files: [./modify_body_two_libs.libr1.ets, ./modify_body_two_libs.libr2.ets]
---*/

import { bar01 } from "./modify_body_two_libs.libr1.ets";
import { baz01 } from "./modify_body_two_libs.libr2.ets";

function main(): void {
    arktest.assertEQ(bar01(), 10, "Function from lib1");
    arktest.assertEQ(baz01(), 20, "Function from lib2");
}
