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

{% for t in cases %}
/*---
desc: >-
    The cast operator may specify a type that is incompatible with
    the class of the object referenced by the run-time value of the
    operand expression.
---*/

interface I {}
interface J {}
interface K {}
class A implements I {}
class B extends A implements J {}

function main(): int {
    {{t.init}};
    try {
      {{t.expr}};
      return {{t.fail}};
    } catch (e: ClassCastError) {
      return 1 - {{t.fail}};
    } catch (e) {
      return 1;
    }
}
{% endfor %}
