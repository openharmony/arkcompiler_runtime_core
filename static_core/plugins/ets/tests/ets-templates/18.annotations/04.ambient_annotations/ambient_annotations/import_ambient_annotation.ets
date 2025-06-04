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
desc: Ambient declaration does not introduce a new annotation but provides type information
      to use an annotation that must be defined elsewhere
files: ['./ambient_annotations.d.ets', './ambient_annotations.ets']
---*/

import { SomeAnnotation{{ idx }} } from "./ambient_annotations.d.ets"

@SomeAnnotation{{ idx }}
class SomeClass{{ idx }} {}

function main(): void {
    let sc{{ idx }}: SomeClass{{ idx }} = new SomeClass{{ idx }}
}
{% endfor %}
