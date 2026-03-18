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

/*---
desc: >-
    The predefined value types are called primitive types.
    Primitive types have are aliases for the classes.
---*/

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Primitive_types_aliases");

    {% for p in primitives %}
        testSuite.addTest("alias_type_{{loop.index}}", () => {
            let a{{loop.index}}: {{p[1]}} = {{p[2]}}
            let b{{loop.index}}: {{p[0]}} = a{{loop.index}}
            arktest.assertEQ(typeof a{{loop.index}}, typeof b{{loop.index}})
            arktest.assertEQ(a{{loop.index}}, b{{loop.index}})
    
            let i{{loop.index}}: {{p[0]}} = {{p[2]}}
            let j{{loop.index}}: {{p[1]}} = i{{loop.index}}
            arktest.assertEQ(typeof i{{loop.index}}, typeof j{{loop.index}})
            arktest.assertEQ(i{{loop.index}}, j{{loop.index}})

            let f{{loop.index}}: {{p[0]}} = {{p[2]}}
            let v{{loop.index}}: {{p[1]}}
            v{{loop.index}} = f{{loop.index}}
            arktest.assertEQ(f{{loop.index}}, v{{loop.index}})
        });
    {%- endfor %}

    return testSuite.run();
}
