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
desc: The base URL setting is used to resolve a path that starts with /
files:
- module_a.ets
- nesteddir/module_a.ets
- nesteddir/../module_a.ets
- ../module_a.ets
name: 13.compilation_units/03.import_directives/06.import_path/file/import_path_file_2
params: import * as all from "./nesteddir/module_a"
---*/

import * as all from "/plugins/ets/tests/ets-templates/13.compilation_units/03.import_directives/06.import_path/file/nesteddir/module_a.ets"

function main(): void {
  all.importedFromFile();
}
