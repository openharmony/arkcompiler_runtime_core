/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
 desc: 13.03 Import Directives
 name: 13.compilation_units/13.03.import_directives/13.03.06.import_path/import_path_03
 tags: 
 files: ["./to_export.ets"]
 ---*/

import {Foo} from "spec/import-export/03.import_directives/06.import_path/to_export" 

function main () {
  let foo = new Foo ("non-relative output")
  console.log(foo)
}
