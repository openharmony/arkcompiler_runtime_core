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
desc: |-
    Export 2 default entities from the same module, one entity imported from another module.
    Only one default export is allowed in a module.
tags: [compile-only]
files:
- ../../07.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_variable.ets
---*/

import test_var from "../../07.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_variable"

export test_var;
export default let i: string = test_var;
