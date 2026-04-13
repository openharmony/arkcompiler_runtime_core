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
    The following soft keywords have special meaning in certain contexts but are valid identifiers elsewhere
    keywords: 
    {%- for kwd in keywords %}
    {{kwd}} 
    {% endfor %}
    Three forms: original, uppercase, capitalized (i.e. out, OUT, Out)
---*/

{%- for kwd in keywords %}
let {{kwd}} = 0;
{{kwd}} = 1
let {{kwd | upper}} = 0;
{{kwd | upper}} = 2
{%- if kwd not in ['readonly', 'set', 'type'] %}
let {{kwd | capitalize}} = 0;
{{kwd | capitalize}} = 3
{%- endif %}

class A{{loop.index}} {
    {{kwd}} = 1
    {{kwd | upper}} = 2
    {%- if kwd not in ['readonly', 'set', 'type'] %}
    {{kwd | capitalize}} = 3
    {%- endif %}
}

class B{{loop.index}} {
    readonly {{kwd}} = 1
    readonly {{kwd | upper}} = 2
    {%- if kwd not in ['readonly', 'set', 'type'] %}
    readonly {{kwd | capitalize}} = 3
    {%- endif %}
}

interface I{{loop.index}} {
    test({{kwd}}: String): String;
    test2({{kwd | upper}}: String): String;
    {%- if kwd not in ['readonly', 'set', 'type'] %}
    test3({{kwd | capitalize}}: String): String;
    {%- endif %}
}

class C{{loop.index}} implements I{{loop.index}} {
    test({{kwd}}: String): String {
        return {{kwd}};
    }
    test2({{kwd | upper}}: String): String {
        return {{kwd | upper}};
    }
    {%- if kwd not in ['readonly', 'set', 'type'] %}
    test3({{kwd | capitalize}}: String): String {
        return {{kwd | capitalize}};
    }
    {%- endif %}
}

abstract class AD{{loop.index}} {
    readonly {{kwd}} = 1
    abstract test({{kwd}}: String): String;
    readonly {{kwd | upper}} = 2
    abstract test2({{kwd | upper}}: String): String;
    {%- if kwd not in ['readonly', 'set', 'type'] %}
    readonly {{kwd | capitalize}} = 3
    abstract test3({{kwd | capitalize}}: String): String;
    {%- endif %}
}

class D{{loop.index}} extends AD{{loop.index}} {
    test({{kwd}}: String): String {
        return {{kwd}};
    }
    test2({{kwd | upper}}: String): String {
        return {{kwd | upper}};
    }
    {%- if kwd not in ['readonly', 'set', 'type'] %}
    test3({{kwd | capitalize}}: String): String {
        return {{kwd | capitalize}};
    }
    {%- endif %}
}

function forCall{{loop.index}}() {
    for(let {{kwd}}: number = 1; {{kwd}} < 2 ; {{kwd}}++ ) {
        let l1 = {{kwd}}
    }
    for(let {{kwd | upper}}: number = 1; {{kwd | upper}} < 2 ; {{kwd | upper}}++) {
        let l2 = {{kwd | upper}}
    }
    {%- if kwd not in ['readonly', 'set', 'type'] %}
    for(let {{kwd | capitalize}}: number = 1; {{kwd | capitalize}} < 2 ; {{kwd | capitalize}}++) {
        let l3 = {{kwd | capitalize}}
    }
    {%- endif %}
    return 12;
}

function forOfCall{{loop.index}}() {
    let x: string = 'The quick brown fox jumps over the lazy dog'
    let letter = ''

    for(let {{kwd}} of x) {
        letter += {{kwd}}
        break;
    }
    for(let {{kwd | upper}} of x) {
        letter += {{kwd | upper}}
        break;
    }
    {%- if kwd not in ['readonly', 'set', 'type'] %}
    for(let {{kwd | capitalize}} of x) {
        letter += {{kwd | capitalize}}
        break;
    }
    return letter == 'TTT' ? 12 : 0
    {% else %}
    return letter == 'TT' ? 12 : 0
    {%- endif %}
    
}

function localDef{{loop.index}}() {
    let {{kwd}}: boolean = true;
    let {{kwd | upper}}: boolean = false;
    {%- if kwd not in ['readonly', 'set', 'type'] %}
    let {{kwd | capitalize}}: boolean = true;
    return {{kwd}} || {{kwd | upper}} && {{kwd | capitalize}}
    {%else %}
    return {{kwd}} || {{kwd | upper}}
    {%- endif %}
}
{%- endfor %}

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Soft_keywords");

    {%- for kwd in keywords %}
    testSuite.addTest("Soft_keyword_{{kwd}}_and_cycles", () => {
        arktest.assertEQ({{kwd}}, 1)
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

        arktest.assertEQ(12, forCall{{loop.index}}())
        arktest.assertEQ(12, forOfCall{{loop.index}}())
        arktest.assertTrue(localDef{{loop.index}}())
    });

    //uppercase keyword test case (var -> VAR)
    testSuite.addTest("Soft_keyword_{{kwd | upper }}", () => {
        arktest.assertEQ({{kwd | upper }}, 2)
        let ca = new A{{loop.index}}()
        ca.{{kwd | upper }} = 2
        arktest.assertEQ(ca.{{kwd | upper }}, 2)
        let cb = new B{{loop.index}}() 
        arktest.assertEQ(cb.{{kwd | upper }}, 2)
        let cc = new C{{loop.index}}()
        arktest.assertEQ(cc.test("test"), "test")
        let cd = new D{{loop.index}}()
        arktest.assertEQ(cd.test("test"), "test")
        arktest.assertEQ(cd.{{kwd | upper }}, 2)
    });
    
    {%- if kwd not in ['readonly', 'set', 'type'] %} //Readonly, Set, Type are 'hard' - defined classes
    //capitalized keyword test case (var -> Var)
    testSuite.addTest("Soft_keyword_{{kwd | capitalize}}", () => {
        arktest.assertEQ({{kwd | capitalize}}, 3)
        let ca = new A{{loop.index}}()
        ca.{{kwd | capitalize}} = 2
        arktest.assertEQ(ca.{{kwd | capitalize}}, 2)
        let cb = new B{{loop.index}}() 
        arktest.assertEQ(cb.{{kwd | capitalize}}, 3)
        let cc = new C{{loop.index}}()
        arktest.assertEQ(cc.test("test"), "test")
        let cd = new D{{loop.index}}()
        arktest.assertEQ(cd.test("test"), "test")
        arktest.assertEQ(cd.{{kwd | capitalize}}, 3)
    });
    {%- endif %}
    {%- endfor %}

    return testSuite.run();
}
