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

/*---
desc: >-
    For an annotation with retention policy (see Retention Annotation) BYTECODE or RUNTIME 
    an abstract class with the name of the annotation is implicitly declared.
    The only way to get instance of an implicitly declared abstract class is to call the reflection library.
---*/

@Retention({{ p.decl }})
@interface ClassAnnotation {
    name: string = "Bob"
} 

@ClassAnnotation({name: "Odenkirk"})
class C {}

type t = Type.from<ClassAnnotation>

@ClassAnnotation({name: "Test"})
type d = Type.from<ClassAnnotation>

function foo(i: ClassAnnotation) {
  return i.name
}

function main() {
    let i: ClassAnnotation = new C()
    arktest.assertEQ(foo(i), "Odenkirk")
    let j: t = {}
    arktest.assertEQ(foo(j), "Bob")
    let k: d = {}
    arktest.assertEQ(foo(k), "Test")
}

{% endfor %}
