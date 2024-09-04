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
    The type of a standalone expression (like just a + b) can be determined entirely
    from the content of the expression; the type of any other expression may be
    influenced by its target type
---*/

function foo(a: byte): int {
  return 1;
}
function foo(a: short): int {
  return 2;
}
function foo(a: char): int {
  return 3;
}
function foo(a: int): int {
  return 4;
}
function foo(a: long): int {
  return 5;
}
function foo(a: float): int {
  return 6;
}
function foo(a: double): int {
  return 7;
}
function foo(a: boolean): int {
  return 8;
}
function foo(a: Byte): int {
  return 21;
}
function foo(a: Short): int {
  return 22;
}
function foo(a: Char): int {
  return 23;
}
function foo(a: Int): int {
  return 24;
}
function foo(a: Long): int {
  return 25;
}
function foo(a: Float): int {
  return 26;
}
function foo(a: Double): int {
  return 27;
}
function foo(a: Boolean): int {
  return 28;
}
function foo(a: Integral): int {
  return 29;
}
function foo(a: Floating): int {
  return 30;
}
function foo(a: String): int {
  return 31;
}
function foo(a: Object): int {
  return 32;
}
function foo(a: byte[]): int {
  return 101;
}
function foo(a: short[]): int {
  return 102;
}
function foo(a: char[]): int {
  return 103;
}
function foo(a: int[]): int {
  return 104;
}
function foo(a: long[]): int {
  return 105;
}
function foo(a: float[]): int {
  return 106;
}
function foo(a: double[]): int {
  return 107;
}
function foo(a: boolean[]): int {
  return 108;
}
function foo(a: Byte[]): int {
  return 121;
}
function foo(a: Short[]): int {
  return 122;
}
function foo(a: Char[]): int {
  return 123;
}
function foo(a: Int[]): int {
  return 124;
}
function foo(a: Long[]): int {
  return 125;
}
function foo(a: Float[]): int {
  return 126;
}
function foo(a: Double[]): int {
  return 127;
}
function foo(a: Boolean[]): int {
  return 128;
}
function foo(a: Integral[]): int {
  return 129;
}
function foo(a: Floating[]): int {
  return 130;
}
function foo(a: String[]): int {
  return 131;
}
function foo(a: Object[]): int {
  return 132;
}
{{c.defs}}

function main(): int {
  if (foo({{c.exp}}) == {{c.res}}) return 0;
  return 1;
}
{% endfor %}
