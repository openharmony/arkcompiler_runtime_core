/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

{% for stmt in statements %}

/*---
desc: Negative tests for different set of statements before calling super()/this()
assert: Call to 'super'/'this' must be first statement in constructor
params: {{stmt}}
tags: [compile-only, negative]
---*/

class A {

    protected parentField: int;

    protected parentMethod(): int {
       return 1;
    }

    public constructor() {

    }

    public constructor(value: String) {

    }

}

class B extends A {
    childField1: int
    childField2: String

    childMethod1() : int {
        return this.childField1;
    }

    childMethod2() : String {
        return this.childField2;
    }

    public constructor(value: String) {

    }
    
    public constructor(value: int) {

    }

    public constructor() {
        {{stmt}}
    }
}

{% endfor %}
