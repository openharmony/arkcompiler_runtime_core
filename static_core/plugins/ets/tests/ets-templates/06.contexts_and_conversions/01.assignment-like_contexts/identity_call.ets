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

{% for c in cases %}
/*---
desc: Invocation contexts allow an identity conversion.
---*/

type Barfunc = (a: Object) => Object;

function bar(a: Object): Object {
    return new Object();
}

class A {
  {%- for t in c['types'] %}
  public fld_{{loop.index}}: {{t.type|safe}};
  {%- endfor %}

  public constructor(
    {%- for t in c['types'] %}
    p{{loop.index}}: {{t.type|safe}}{{ ", " if not loop.last }}
    {%- endfor %}
  ) {
    {%- for t in c['types'] %}
    this.fld_{{loop.index}} = p{{loop.index}};
    {%- endfor %}
  }

  public foo(
    {%- for t in c['types'] %}
    p{{loop.index}}: {{t.type|safe}}{{ ", " if not loop.last }}
    {%- endfor %}
  ): int {
    if (
      {%- for t in c['types'] %}
      this.fld_{{loop.index}} {{t.op}} p{{loop.index}}{{ " ||" if not loop.last }}
      {%- endfor %}
    ) {
      return 1;
    } else {
      return 0;
    }
  }
}

function main(): int {
  let c: A = new A({% for t in c['types'] %}{{t.expr|safe}}{{ ", " if not loop.last }}{% endfor %});
  return c.foo({% for t in c['types'] %}c.fld_{{loop.index}}{{ ", " if not loop.last }}{% endfor %});
}
{% endfor %}
