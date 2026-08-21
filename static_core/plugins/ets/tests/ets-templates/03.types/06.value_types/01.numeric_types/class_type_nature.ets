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

/*---
desc: >-
    Numeric types are of the class type nature, ...all of them are subtypes of Object,
    and therefore can be used at any place where a class name is expected.
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite('Numeric types as objects')

{%- for type in ['byte', 'short', 'int', 'long', 'float', 'double', 'number', 'Byte', 'Short', 'Int', 'Long', 'Float', 'Double', 'Number'] %}
  suite.addTest('Type {{type}}', () => {
      let v0 = new {{type}};
      arktest.assertEQ(v0, 0, 'expected: 0 for new {{type}}');
      let v1 = new {{type}}();
      arktest.assertEQ(v1, 0, 'expected: 0 for new {{type}}()');
      let v2 = new {{type}}(1);
      arktest.assertEQ(v2, 1, 'expected: 1 for new {{type}}(1)');
      let v3 = new {{type}}(-1);
      arktest.assertEQ(v3, -1, 'expected: -1 for new {{type}}(-1)');
  })
{% endfor %}
  return suite.run()
}
