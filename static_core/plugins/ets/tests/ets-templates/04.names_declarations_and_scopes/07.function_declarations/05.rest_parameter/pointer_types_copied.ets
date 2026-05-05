/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Any call to a function, method, constructor, or lambda with a rest parameter implies that a new array or tuple is created from the arguments provided.
---*/

// function
let arg_fn: {{c.arg_type}} = {{c.arg_value}}
function foo(...rest_parameter: {{c.arg_type}}) {
    rest_parameter[0] = 1234
}
function pass(...rest_parameter: {{c.arg_type}}): {{c.arg_type}} {
    return rest_parameter
}
foo(...arg_fn)
let op: {{c.arg_type}} = pass(...arg_fn)
arktest.assertNE(arg_fn, op)
arktest.assertEQ(arg_fn[0], 1, `The function call with a rest parameters modified the original {{c.type_hint}} provided in argument`)

// class static method
let arg_mtd: {{c.arg_type}} = {{c.arg_value}}
class A {
    static foo(...rest_parameter: {{c.arg_type}}): void {
        rest_parameter[0] = 1234
    }
    static pass(...rest_parameter: {{c.arg_type}}): {{c.arg_type}} {
        return rest_parameter
    }
}
A.foo(...arg_mtd)
op = A.pass(...arg_mtd)
arktest.assertNE(arg_mtd, op)
arktest.assertEQ(arg_mtd[0], 1, `The method call with a rest parameters modified the original {{c.type_hint}} provided in argument`)

// class method
arg_mtd = {{c.arg_value}}
class C {
    foo(...rest_parameter: {{c.arg_type}}): void {
        rest_parameter[0] = 1234
    }
    pass(...rest_parameter: {{c.arg_type}}): {{c.arg_type}} {
        return rest_parameter
    }
}
let a: C = new C();
a.foo(...arg_mtd)
op = a.pass(...arg_mtd)
arktest.assertNE(arg_mtd, op)
arktest.assertEQ(arg_mtd[0], 1, `The method call with a rest parameters modified the original {{c.type_hint}} provided in argument`)


// constructor
let arg_ctr: {{c.arg_type}} = {{c.arg_value}}
class B {
    constructor(...rest_parameter: {{c.arg_type}}) {
        rest_parameter[0] = 1234
    }
}
let b = new B(...arg_ctr)
arktest.assertEQ(arg_ctr[0], 1, `The class constructor call with a rest parameters modified the original {{c.type_hint}} provided in argument`)

// lambda
let argument4: {{c.arg_type}} = {{c.arg_value}};
(...rest_parameter: {{c.arg_type}}) => {rest_parameter[0] = 1234}(...argument4);
arktest.assertEQ(argument4[0], 1, `The lambda function call with a rest parameters modified the original {{c.type_hint}} provided in argument`)


// interface method
arg_mtd = {{c.arg_value}}
interface IRest {
    foo(...rest_parameter: {{c.arg_type}}): void {
        rest_parameter[0] = 1234
    }
    pass(...rest_parameter: {{c.arg_type}}): {{c.arg_type}} {
        return rest_parameter
    }
}

class D implements IRest {}

let d: D = new D();
d.foo(...arg_mtd)
op = d.pass(...arg_mtd)
arktest.assertNE(arg_mtd, op)
arktest.assertEQ(arg_mtd[0], 1, `The method call with a rest parameters modified the original {{c.type_hint}} provided in argument`)


{% endfor %}
