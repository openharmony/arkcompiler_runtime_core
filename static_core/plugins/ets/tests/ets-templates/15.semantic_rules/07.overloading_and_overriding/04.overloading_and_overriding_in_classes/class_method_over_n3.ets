/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

{% for c in cases %}
/*---
desc: >-
    An instance method is defined in a subclass with the same name as the instance method in a superclass.
    If signatures are override-compatible (see Override-Compatible Signatures), then overriding is used. Otherwise,
    overloading is used.

    A static method is defined in a subclass with the same name as the static method in a superclass.
    If signatures are overload-equivalent (see Overload-Equivalent Signatures), then the static method in the
    subclass hides the previous static method. Otherwise, overloading is used.

    {{c.doc|indent}}
tags: [{{c.tags}}]
---*/

let res: number = 0;

{{c.decl}}

function main(): void {
  {{c.use}}
}

{% endfor %}
