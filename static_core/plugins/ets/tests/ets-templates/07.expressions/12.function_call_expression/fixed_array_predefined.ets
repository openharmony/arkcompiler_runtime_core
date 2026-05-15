/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

{% for t in types %}
/*---
desc: Verify behavior when a function has at least one parameter or return type of the type FixedArray
   parameterized with a type parameter and function call expression leads to instantiation of the type
   FixedArray with the predefined element type
---*/

function foo<T>(p: FixedArray<T>): T {
    return p[0]
}

function foo1<T>(p: FixedArray<T>): FixedArray<T> {
    return p
}

function foo2<T>(...p: FixedArray<T>): FixedArray<T> {
    return p
}

function change<T>(v: T, id: int, p: FixedArray<T>): T {
    p[id] = v
    return p[id]
}

function foo1_caller(): FixedArray<{{t.type}}> {
    let p: {{t.type}} = {{t.init}}
    let fa: FixedArray<{{t.type}}> = [p]
    return foo1<{{t.type}}>(fa)
}

function foo2_caller(): FixedArray<{{t.type}}> {
    let p: {{t.type}} = {{t.init}}
    return foo2<{{t.type}}>(p)
}

function test_param() {
    let foo_var = foo<{{t.type}}>
    let param: FixedArray<{{t.type}}> = [{{t.init}}]
    let res = foo_var(param)
    arktest.assertTrue(res instanceof {{t.type}}, "expect instanceof {{t.type}}")
    arktest.assertEQ(res, {{t.init}})
}

function test_return() {
    let foo_res = foo1_caller()
    arktest.assertTrue(foo_res instanceof FixedArray<{{t.type}}>,
      "expect instanceof FixedArray<{{t.type}}>")
    arktest.assertEQ(foo_res.length, 1)
    arktest.assertTrue(foo_res[0] instanceof {{t.type}}, "expect instanceof {{t.type}}")
    arktest.assertEQ(foo_res[0], {{t.init}})
}

function test_return_from_function_with_rest_parameter() {
    let foo_res = foo2_caller()
    arktest.assertTrue(foo_res instanceof FixedArray<{{t.type}}>,
      "expect instanceof FixedArray<{{t.type}}>")
    arktest.assertEQ(foo_res.length, 1)
    arktest.assertTrue(foo_res[0] instanceof {{t.type}}, "expect instanceof {{t.type}}")
    arktest.assertEQ(foo_res[0], {{t.init}})
}

function test_change() {
    let foo_var = change<{{t.type}}>
    let param: FixedArray<{{t.type}}> = [{{t.init}}]
    let res = foo_var({{t.change}}, 0, param)
    arktest.assertTrue(res instanceof {{t.type}}, "expect instanceof {{t.type}}")
    arktest.assertEQ(res, {{t.change}})
    arktest.assertTrue(param[0] instanceof {{t.type}}, "expect instanceof {{t.type}}")
    arktest.assertEQ(param[0], {{t.change}})
}

function main() {
    let testSuite = new arktest.ArkTestsuite('fixed_array_predefined')
    testSuite.addTest('test_param', test_param)
    testSuite.addTest('test_return', test_return)
    testSuite.addTest('test_return_from_function_with_rest_parameter', test_return_from_function_with_rest_parameter)
    testSuite.addTest('test_change', test_change)
    return testSuite.run()
}

{% endfor %}
