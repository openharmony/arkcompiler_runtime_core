/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
{%yaml "readonly_field_class.not_covariant_function_subtype.test_data.yaml" as td %}
{% for c, err in td.list_function_type_error.items() %}
/*---
desc: Checking for initialization with different not covariant function subtype values usage. (initialization static field, '{{c}}').
expected_out: >- 
    {{err}}
tags: [compile-only, negative]
---*/

class A {}
class B extends A {}
class C extends A {}
type HandlerReq = (a: B, b: int) => B
const handlerReq: HandlerReq = (a: B, b: int): B => {
    return new B
}
type HandlerCInt = (a: C, b: int) => B
const handlerCInt: HandlerCInt = (a: C, b: int): B => {
    return new B
}
type HandlerBInt = (a: B, b: int) => A
const handlerBInt: HandlerBInt = (a: B, b: int): A => {
    return new B
}
type HandlerB = (a: B) => A
const handlerB: HandlerB = (a: B): A => {
    return new B
}
type HandlerBIntInt = (a: B, b: int, c: int) => B
const handlerBIntInt: HandlerBIntInt = (a: B, b: int, c: int): B => {
    return new B
}
type HandlerNothing = () => C
const handlerNothing: HandlerNothing = (): C => {
    return new C
}
class D {
    static readonly field: HandlerReq = {{c}}
}

{% endfor %}