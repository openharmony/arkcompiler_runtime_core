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

/*---
  files: [imported_module_1.ets, imported_module_2.ets]
---*/

import {foo1, foo1} from './imported_module_1.ets'
import {foo2With1param as foo2With1param} from './imported_module_2.ets'


function main() : void {
    arktest.assertEQ(foo1(), 0);
    arktest.assertEQ(foo2With1param(1), 1);
}
