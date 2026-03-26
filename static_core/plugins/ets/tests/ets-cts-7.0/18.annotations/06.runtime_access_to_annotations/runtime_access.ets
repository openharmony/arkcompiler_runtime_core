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

{% for case in cases %}

/*---
desc: >-
    For an annotation with retention policy (see Retention Annotation) BYTECODE or RUNTIME 
    an abstract class with the name of the annotation is implicitly declared. 
    All fields of this class are readonly. If a field is of an array type, the array type is also readonly
---*/

@Retention({{ case.decl }})
@interface ClassAnnotation {
    name: string
    attrs: number[]
    isActual: boolean
} 

@ClassAnnotation({name: "testName", attrs: [1, 2.0], isActual = true})
class MyClass {}

function main() {
    let my: ClassAnnotation
    //let my: Type = Type.from<MyClass>()
}

{% endfor %}
