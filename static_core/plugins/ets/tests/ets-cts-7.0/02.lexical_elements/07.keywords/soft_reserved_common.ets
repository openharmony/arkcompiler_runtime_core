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
    The following identifiers are also treated as soft keywords reserved for the future use,
    or currently used in TypeScript
    keywords:
    {%- for kwd in keywords %}
    {{kwd}}  
    {% endfor %}
    Three forms: original, uppercase, capitalized (i.e. memo, MEMO, Memo)
---*/

{% for kwd in keywords %}
{%- if kwd not in ['var'] %}
let {{kwd}} = 1;
const {{kwd | upper}} = 2
let {{kwd | capitalize}} = 3
{% else %}
//"var X" is guarded by syntax checker (ESY0297: 'var' keyword is not supported. Use 'let' instead.)
{%- endif %}

{%- if kwd not in ['struct'] %}
class A{{loop.index}} {
    {{kwd}} = 1
    {{kwd | upper}} = 2
    {{kwd | capitalize}} = 3
}

class B{{loop.index}} {
    readonly {{kwd}} = 1
    readonly {{kwd | upper}} = 2
    readonly {{kwd | capitalize}} = 3
}

interface I{{loop.index}} {
    test({{kwd}}: String): String; 
    test2({{kwd | upper}}: String): String;
    test3({{kwd | capitalize}}: String): String; 
}

class C{{loop.index}} implements I{{loop.index}} {
    test({{kwd}}: String): String{
        return {{kwd}};
    }
    test2({{kwd | upper}}: String): String{
        return {{kwd | upper}};
    }
    test3({{kwd | capitalize}}: String): String{
        return {{kwd | capitalize}};
    }
}

abstract class AD{{loop.index}} {
    readonly {{kwd}} = 1
    abstract test({{kwd}}: String): String;
    readonly {{kwd | upper}} = 2
    abstract test2({{kwd | upper}}: String): String;
    readonly {{kwd |capitalize}} = 3
    abstract test3({{kwd | capitalize}}: String): String;
}

class D{{loop.index}} extends AD{{loop.index}} {
    test({{kwd}}: String): String{
        return {{kwd}};
    }
    test2({{kwd | upper}}: String): String{
        return {{kwd | upper}};
    }
    test3({{kwd | capitalize}}: String): String{
        return {{kwd | capitalize}};
    }
}
{% else %}
//"struct" is an alias to class (20.6 chapter - Keyword struct and ArkUI)
{%- endif %}
{% endfor %}


function main(): int {

    let testSuite = new arktest.ArkTestsuite("Soft_keywords_reserved");

    {%- for kwd in keywords %}
    testSuite.addTest("keyword_{{kwd}}", () => {
        {%- if kwd not in ['var'] %}
        arktest.assertEQ({{kwd}}, 1)
        {%- endif %}

        {%- if kwd not in ['struct'] %}
        let ca = new A{{loop.index}}()
        ca.{{kwd}} = 2
        arktest.assertEQ(ca.{{kwd}}, 2)
        let cb = new B{{loop.index}}() 
        arktest.assertEQ(cb.{{kwd}}, 1)
        let cc = new C{{loop.index}}()
        arktest.assertEQ(cc.test("test"), "test")
        let cd = new D{{loop.index}}()
        arktest.assertEQ(cd.test("test"), "test")
        arktest.assertEQ(cd.{{kwd}}, 1)
        {%- endif %}
        });

        //uppercase keyword test case (var -> VAR)
        testSuite.addTest("keyword_{{kwd | upper }}_uppercase", () => {
        {%- if kwd not in ['var'] %}
        arktest.assertEQ({{kwd | upper}}, 2)
        {%- endif %}

        {%- if kwd not in ['struct'] %}
        let ca = new A{{loop.index}}()
        ca.{{kwd | upper}} = 2
        arktest.assertEQ(ca.{{kwd | upper}}, 2)
        let cb = new B{{loop.index}}() 
        arktest.assertEQ(cb.{{kwd | upper}}, 2)
        let cc = new C{{loop.index}}()
        arktest.assertEQ(cc.test2("test"), "test")
        let cd = new D{{loop.index}}()
        arktest.assertEQ(cd.test2("test"), "test")
        arktest.assertEQ(cd.{{kwd | upper}}, 2)
        {%- endif %}
        });

        //capitalized keyword test case (var -> Var)
        testSuite.addTest("keyword_{{kwd | capitalize}}_capitalize", () => {
        {%- if kwd not in ['var'] %}
        arktest.assertEQ({{kwd | capitalize}}, 3)
        {%- endif %}

        {%- if kwd not in ['struct'] %}
        let ca = new A{{loop.index}}()
        ca.{{kwd | capitalize}} = 3
        arktest.assertEQ(ca.{{kwd | capitalize}}, 3)
        let cb = new B{{loop.index}}() 
        arktest.assertEQ(cb.{{kwd | capitalize}}, 3)
        let cc = new C{{loop.index}}()
        arktest.assertEQ(cc.test3("test"), "test")
        let cd = new D{{loop.index}}()
        arktest.assertEQ(cd.test3("test"), "test")
        arktest.assertEQ(cd.{{kwd | capitalize}}, 3)
        {%- endif %}
        });
    {%- endfor %}

    return testSuite.run();
}
