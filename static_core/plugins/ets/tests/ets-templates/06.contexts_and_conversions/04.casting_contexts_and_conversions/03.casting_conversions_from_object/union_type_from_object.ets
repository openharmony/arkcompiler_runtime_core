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

{% for c in cases %}
/*---
desc: Casting conversion from Object to union.
---*/

class Cat { name(): string { return "Tom" }; meow (): string { return 'M'} }
class Dog { name(): string { return "Sam" }; bark (): string { return 'B'} }
class Pet { name(): string { return "Bro" }; leap (): string { return 'L'} }

type Animal = Cat | Dog | Pet

function convertObject(obj: Object): {{c.type}} {
   return obj as {{c.type}}
}

function main() {
    let animal: Animal = new {{c.animal}}()
    let obj: Object = animal as Object
    let target: {{c.type}} = convertObject(obj)
    assertEQ(target.{{c.call}}(), "{{c.expected}}")
}
{% endfor %}
