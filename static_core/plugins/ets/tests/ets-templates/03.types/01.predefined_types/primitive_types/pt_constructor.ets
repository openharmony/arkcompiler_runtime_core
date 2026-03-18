/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
    Primitives are class types that are a part of the Standard Library. It means that they are subtypes of
    Object, and therefore can be used at any place where a class name is expected
---*/

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Primitive_types_constructors");

    {% for p in primitives %}
        testSuite.addTest("constructor_{{loop.index}}", () => {
            new {{p.stype}}
            new {{p.stype}}()
        });
    {%- endfor %}

    return testSuite.run();
}
