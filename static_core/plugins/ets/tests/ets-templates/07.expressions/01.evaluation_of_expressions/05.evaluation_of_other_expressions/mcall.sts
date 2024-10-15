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

/*---
desc: >-
    The choice of the method of o.m(...)call depends on the methods
    comprising the class or interface of the o type.
---*/

interface I {
    meth(s: String): String;
}

class A implements I {
  public override meth(s: String): String {
      return s + "A";
  }
}

class B extends A {
  public override meth(s: String): String {
      return super.meth(s) + "B";
  }
}

function main(): int {
    let a: A = new A();
    if (!a.meth("X").equals("XA")) return 1;

    a = new B();
    if (!a.meth("Y").equals("YAB")) return 1;

    let i: I = new A();
    if (!i.meth("Z").equals("ZA")) return 1;

    i = new B();
    if (!i.meth("W").equals("WAB")) return 1;

    return 0;
}
