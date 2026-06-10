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

{% yaml "../generic_params.params.yaml" as yaml_file %}
{% for k in yaml_file.soft_keywords %}
/*---
desc: >-
    Syntax-wise, a type parameter is an unqualified identifier with a proper scope.
    Soft keywords have special meanings reserved in certain contexts. However, soft
    keywords are valid identifiers.
---*/

interface I<{{k.keyword}}> {
    getId() { return 27 }
}

function main() {
    let i: I<Object> = {}
    arktest.assertEQ(i.getId(), 27)
}
{% endfor %}
