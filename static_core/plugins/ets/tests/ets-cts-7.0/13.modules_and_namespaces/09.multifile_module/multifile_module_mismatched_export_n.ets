/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
    If two module headers have the same moduleName but different export
    modifiers, then a compile-time error occurs.
tags: [compile-only, negative]
files:
- ./modules/mod_export.ets
- ./modules/mod_no_export.ets
expected_out: "different export modifiers"
---*/

import { foo } from "./modules/mod_export"
import { bar } from "./modules/mod_no_export"
