/*
 * Copyright (c)2025 Huawei Device Co., Ltd.
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
    public bar(v: int) {
        return v
    }

    public lambda = () => 1

    public static staticBar () {}
}

interface B {
    buzz()
}

class C implements B {
    buzz() {}
}


function foo(v: int) {
    return v
};

function returnFunctionName(a: Function): string {
    return a.name
}

function main() {
    let x = () => {}
    let a: A = new A()
    let b: B = new C()
    let def = true ? () => { } : null

    assertEQ(foo.name, "foo")
    assertEQ(x.name, "x")
    assertEQ(a.bar.name, "bar")
    assertEQ(a.lambda.name, "lambda")
    assertEQ(A.staticBar.name, "staticBar")
    assertEQ(b.buzz.name, "buzz")
    assertEQ(def?.name, "")

    assertEQ(returnFunctionName(foo), "foo")
    assertEQ(returnFunctionName(x), "x")
    assertEQ(returnFunctionName(a.bar), "bar")
    assertEQ(returnFunctionName(a.lambda), "lambda")
    assertEQ(returnFunctionName(A.staticBar), "staticBar")
    assertEQ(returnFunctionName(b.buzz), "buzz")
    assertEQ(returnFunctionName(() => 1), "")
    if (def) {
        assertEQ(returnFunctionName(def), "")
    }
}
