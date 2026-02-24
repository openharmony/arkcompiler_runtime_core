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

/*---
desc: Correct number literals with bases 16, 10, 8 and 2
---*/

{% for c in cases %}
function testNumberLiteral_{{loop.index}}() {
    let b{{loop.index}} = new Number({{c['lit']}})

    arktest.assertEQ(b{{loop.index}}, {{c['val']}}, "b{{loop.index}}: {{c['lit']}} != {{c['val']}}")
}
{% endfor %}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('Number literals');
    {% for c in cases %}
    testSuite.addTest("Test {{loop.index}}", testNumberLiteral_{{loop.index}})
    {% endfor %}
    return testSuite.run();
}