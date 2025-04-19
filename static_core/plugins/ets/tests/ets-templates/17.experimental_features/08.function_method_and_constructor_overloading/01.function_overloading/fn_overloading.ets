/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

{% for c in cases %}
/*---
desc: >-
    If two functions declared in the same declaration space
    have the same name but different signatures that are not
    override-equivalent, then the function name is said to be overloaded.
params: {{c.type}}
tags: [{{c.tags}}]
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
function foo(a: FixedArray<byte>): int {
  return 101;
}
function foo(a: FixedArray<short>): int {
  return 102;
}
function foo(a: FixedArray<char>): int {
  return 103;
}
function foo(a: FixedArray<int>): int {
  return 104;
}
function foo(a: FixedArray<long>): int {
  return 105;
}
function foo(a: FixedArray<float>): int {
  return 106;
}
function foo(a: FixedArray<double>): int {
  return 107;
}
function foo(a: FixedArray<boolean>): int {
  return 108;
}
function foo(a: FixedArray<Byte>): int {
  return 121;
}
function foo(a: FixedArray<Short>): int {
  return 122;
}
function foo(a: FixedArray<Char>): int {
  return 123;
}
function foo(a: FixedArray<Int>): int {
  return 124;
}
function foo(a: FixedArray<Long>): int {
  return 125;
}
function foo(a: FixedArray<Float>): int {
  return 126;
}
function foo(a: FixedArray<Double>): int {
  return 127;
}
function foo(a: FixedArray<Boolean>): int {
  return 128;
}
function foo(a: FixedArray<Integral>): int {
  return 129;
}
function foo(a: FixedArray<Floating>): int {
  return 130;
}
function foo(a: FixedArray<String>): int {
  return 131;
}
function foo(a: FixedArray<Object>): int {
  return 132;
}

function main(): int {
  let v: {{c.type}} = {{c.val}};
  if (foo(v) == {{c.res}}) return 0;
  return 1;
}
{% endfor %}
