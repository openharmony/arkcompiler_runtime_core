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

{% for idx in range(1,4) %}

/*---
desc: If an annotation is applied to an ambient declaration in the .d.ets file then the annotation is
      to be applied to the implementation declaration manually
files: ['./annotation_ambient_declaration.ets']
---*/

import * as m from "./annotation_ambient_declaration.ets"

let c = new m.SomeClass{{ idx }}

{% endfor %}