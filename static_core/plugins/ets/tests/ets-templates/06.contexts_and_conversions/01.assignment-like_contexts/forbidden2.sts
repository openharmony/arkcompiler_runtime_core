/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

{% for c in cases %}
/*---
desc: >-
    Any conversion that is not explicitly allowed is forbidden.
tags: [negative]
---*/

{{c.defs}}

function main(): int {
  let src: {{c.stype}} = {{c.sval|safe}};
  let dst: {{c.dtype}} = src as {{c.dtype}};
  return 0;   // shouldn't reach this line, ClassCastError is expected
}
{% endfor %}
