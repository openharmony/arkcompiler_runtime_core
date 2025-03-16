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
desc: >-
    Unary numeric promotion is performed on expressions in the following situation:
    Each operand, separately, of a shift operator.
params: {{c.type}}
---*/

function foo(p: byte): int {
  return 1;
}

function foo(p: short): int {
  return 2;
}

function foo(p: int): int {
  return 3;
}

function foo(p: long): int {
  return 4;
}

function foo(p: char): int {
  return 5;
}

function foo(p: boolean): int {
  return 6;
}

function foo(p: float): int {
  return 7;
}

function foo(p: double): int {
  return 8;
}

function foo(p: Byte): int {
  return 21;
}

function foo(p: Short): int {
  return 22;
}

function foo(p: Int): int {
  return 23;
}

function foo(p: Long): int {
  return 24;
}

function foo(p: Char): int {
  return 25;
}

function foo(p: Boolean): int {
  return 26;
}

function foo(p: Float): int {
  return 27;
}

function foo(p: Double): int {
  return 28;
}

function foo(p: Object): int {
  return 40;
}

function main(): int {
  let v: {{c.type}} = {{c.val}};
  {%- for r in rside %}
  let r{{loop.index}}: {{r.type}} = {{r.val}};
  if (foo(v << r{{loop.index}}) != {{c.res}}) {
    return 1;
  }
  if (foo(v >> r{{loop.index}}) != {{c.res}}) {
    return 1;
  }
  if (foo(v >>> r{{loop.index}}) != {{c.res}}) {
    return 1;
  }
  {% endfor %}
  return 0;
}
{% endfor %}
