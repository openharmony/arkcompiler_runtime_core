/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
    Invocation contexts allow a widening reference conversion.
    Widening reference conversions
---*/

interface I {}
interface J extends I {}
class A implements I {}
class B extends A implements J {}
class C extends B {}

{% for c in cases %}
class Clazz{{loop.index}} {
    public fld1: {{c.totype}};
    public fld2: {{c.totype}}|null = null;

    public constructor(p: {{c.totype}}) {
        this.fld1 = p;
    }

    public foo(p: {{c.totype}}): void {
        this.fld2 = p;
    }
}
{%- endfor %}

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Context_widening_call");

    {% for c in cases %}
        testSuite.addTest("call_{{loop.index}}", () => {
            let v: {{c.fromtype}} = {{c.fromval | safe}};
            let obj: Clazz{{loop.index}} = new Clazz{{loop.index}}(v);
            obj.foo(v);
            arktest.assertEQ(obj.fld1, v) 
            arktest.assertEQ(obj.fld2, v)
        }); 
    {% endfor %}

    return testSuite.run();

}
