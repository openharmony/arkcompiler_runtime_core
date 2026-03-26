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

{% for type in types %}

/*---
desc: >-
    Numeric types are of the class type nature, ...all of them are subtypes of Object,
    and therefore can be used at any place where a class name is expected.
---*/

function main(): void {
  let a = new {{type}}
  arktest.assertTrue(a instanceof Object, "expect {{type}} instanceof Object");
  arktest.assertEQ(a, 0, "expect {{type}} 0");
  let a1 = new {{type}}(1)
  arktest.assertEQ(a1, 1, "expect {{type}} 1");
}

{% endfor %}
