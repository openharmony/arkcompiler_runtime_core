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

class A {
  foo(v: String): String { return ("A:" + v) }
}

class B extends A {
  foo(v: String): String { return ("B:" + v) }
}

class C {
  foo(v: String): String { return ("C:" + v) }
}

class D extends C {}

function test1(v: A | C): String {
  return v.foo("123")
}

function test2(v: B | C): String {
  return v.foo("123")
}

function test3(v: B | D): String {
  return v.foo("123")
}

function main() {
  assertEQ(test1(new A()), "A:123");
  assertEQ(test1(new C()), "C:123");
  assertEQ(test2(new B()), "B:123");
  assertEQ(test3(new D()), "C:123");
}
