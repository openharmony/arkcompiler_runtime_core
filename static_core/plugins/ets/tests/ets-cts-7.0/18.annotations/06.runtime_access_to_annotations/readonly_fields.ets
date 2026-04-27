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

{% for p in policies %}
{% for c in cases %}

/*---
desc: >-
    For an annotation with retention policy (see Retention Annotation) BYTECODE or RUNTIME 
    an abstract class with the name of the annotation is implicitly declared. 
    All fields of this class are readonly. If a field is of an array type, the array type is also readonly
tags: [compile-only, negative]
expected_out: "Semantic error ESE4002: Cannot assign to a readonly field f"
---*/

const initVal = {{c.init}}
const editVal = {{c.editVal}}

@Retention({{ p.option }})
@interface ClassAnnotation {
  f: {{c.type}}
}

@ClassAnnotation({ f: initVal })
class C {}

function main() {
    let i = new C()
    i.f{{c.array_index}} = editVal
}

{% endfor %}
{% endfor %}
