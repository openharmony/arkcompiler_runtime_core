/**
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
desc: Custom tuple type declaration and possibility to use in instanceof expression.
---*/

{% for c in cases %}
type MyTuple{{loop.index0}} = {{c.type}}
{% endfor %}

function main(): int {
    let myTestsuite = new arktest.ArkTestsuite("custom_tuple_type_suite");

    {% for c in cases %}
    myTestsuite.addTest("custom_tuple_type_test_{{loop.index0}}", () => {
        let tuple: MyTuple{{loop.index0}} = {{c.value}};
        let arrTuple: Array<MyTuple{{loop.index0}}> = [{{c.value}}];
        arktest.assertTrue(tuple instanceof Tuple2)
        arktest.assertTrue(arrTuple[0] instanceof Tuple2)
        arktest.assertFalse(tuple instanceof Tuple3)
        arktest.assertFalse(arrTuple[0] instanceof Tuple3)
    });
    {% endfor %}
    return myTestsuite.run();
}
