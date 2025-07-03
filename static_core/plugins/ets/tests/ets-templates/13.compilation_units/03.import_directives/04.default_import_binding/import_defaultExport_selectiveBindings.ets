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
/*---
desc: Support is needed at present. 1.import defaultExport,allBinding from importPath 2.import defaultExport,selectiveBindings from importPath
files:
  - ./export_default/single_export.ets
  - ./export_default/selective_export.ets
---*/

import a,{b as b,c as c} from "./export_default/single_export.ets";
import a2,{b as b2,c as c2} from "./export_default/selective_export.ets";

function main(){
    assertEQ(a,100)
    assertEQ(b,10000)
    assertEQ(c,"export_string")

    assertEQ(a2,100)
    assertEQ(b2,10000)
    assertEQ(c2,"export_string")
}
