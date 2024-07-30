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

{% for member in class_members %}

/*---
desc: Use variables that not declare in an inner class, with readonly modifier.
assert: Any local variable, formal parameter, or exception and panic parameter used but not declared in an inner class must be declared readonly, or a compile-time error occurs where the use is attempted.
tags: [compile-only]
---*/

class ZeroDivisor extends Exception { }

class A {
  readonly {{member.decl}};

  public inner class B {
    public foo(): void {
      {{member.call}};
    }

    public foo(y: int): void {
      let b: int = y;
    }
  }
}

{% endfor %}
