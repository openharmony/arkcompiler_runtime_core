/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

{% for c in cases %}

/*---
desc: Variance is used to describe the subtyping operation on parameterized types. Test {{c.desc}}.
tags: [compile-only, {{c.tag}}]
---*/

{{c.class}} A {}
{{c.class}} B extends A {}

class TestClass<{{c.param}}> {}

function main(): void {
    let testClass: TestClass<{{c.type1}}> = new TestClass<{{c.type2}}>
}

{% endfor %}
