/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

{%- for c in cases %}

/*---
desc: |-
    Another important property of type Function is name. It is a string that contains
    the name associated with the function object in the following way:
    - If a function or a method is assigned to a function object, then the associated name
      is that of the function or of the method;
    - If a lambda is assigned to a variable of Function type, then the associated name
      is that of the variable;
    - Otherwise, the string is empty.
---*/

{{c.decl}}

function main() {
    {{c.main|indent}}
}
{%- endfor %}
