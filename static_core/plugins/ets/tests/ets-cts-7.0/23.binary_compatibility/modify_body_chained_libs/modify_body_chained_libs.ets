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
    Chained libs: test imports from lib1, lib1 imports from lib2.
    Only lib1 changes (body modification). Lib2 stays unchanged.
files: [./modify_body_chained_libs.libr1.ets]
---*/

import { calc } from "./modify_body_chained_libs.libr1.ets";

function main(): void {
    arktest.assertEQ(calc(), 30, "Chained call through lib1 -> lib2");
}
