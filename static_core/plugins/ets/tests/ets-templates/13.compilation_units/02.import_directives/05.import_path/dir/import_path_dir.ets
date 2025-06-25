/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

{% for stmt in import_stmts %}

/*---
desc: |- 
  If import path refers to a folder denoted by the last name in the resolved file path, then the compiler imports the
  package which resides in the folder.
tags: [compile-only]
params: {{stmt}}
files: [module_a.ets, module_a/module_a.ets, nesteddir/module_a/module_a.ets]
---*/

{{stmt}}

function main(): void {
  ma.importedFromDir()
}

{% endfor %}
