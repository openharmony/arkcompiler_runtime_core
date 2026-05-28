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
desc: Properly dispatch when static and instance method have same name.
---*/

class A {
    foo(): string { return "instance" };
    static foo(): string { return "static" }
}

class B extends A {
    foo(): string { return "instance B" };
    static foo(): string { return "static B" }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("static vs instance methods");
    {% for case in cases %}
    suite.addTest("test_{{loop.index}}", () => {
        {{case.use|indent}}
    });
    {% endfor %}
    return suite.run()
}
