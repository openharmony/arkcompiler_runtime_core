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
desc: Invocation contexts allow a widening primitive conversion.
params: Widening primitive conversion to '{{c.ctype}}'
---*/

class A {
  public fld1: {{c.ctype}};
  public fld2: {{c.ctype}};

  public constructor(p: {{c.ctype}}) {
    this.fld1 = p;
  }

  public foo(p: {{c.ctype}}): void {
    this.fld2 = p;
  }
}

function main(): int {
  {% for t in c['values'] %}
  let v{{loop.index}}: {{t.type}} = {{t.val|safe}};
  let obj{{loop.index}}: A = new A(v{{loop.index}});
  obj{{loop.index}}.foo(v{{loop.index}});
  if (obj{{loop.index}}.fld1 != {{t.val2|safe}} || obj{{loop.index}}.fld2 != {{t.val2|safe}}) {
    return 1;
  }
  {% endfor %}
  return 0;
}
{% endfor %}
