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

{% for t in cases %}
/*---
desc: >-
    A catch clause can throw an exception only if the class of the thrown
    exception object is an instanceof the type of the formal parameter of
    the respective catch clause.
---*/

interface I {}
interface J {}
interface K {}
class A extends Exception implements I {}
class B extends A implements J {}
class C extends B implements K {}

function main(): int {
    let v: Exception = {{t.init}};
    try {
      throw v;
    } catch (e: {{t.exc}}) {
      return {{t.fail}};
    } catch (e) {
      return 1 - {{t.fail}};
    }
}
{% endfor %}
