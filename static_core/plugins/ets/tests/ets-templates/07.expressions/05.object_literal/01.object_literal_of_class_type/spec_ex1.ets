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

/*---
desc: If the class type C is inferred from the context, then the type of object literal is C.
tags: [compile-only]
---*/

class Person {
    name: string = ""
    age: number = 0
}
function foo(p: Person) { /*some code*/ }
// ...
let p: Person = {name: "Bob", age: 25} /* ok, variable type is
used */
foo({name: "Alice", age: 18}) // ok, parameter type is used
