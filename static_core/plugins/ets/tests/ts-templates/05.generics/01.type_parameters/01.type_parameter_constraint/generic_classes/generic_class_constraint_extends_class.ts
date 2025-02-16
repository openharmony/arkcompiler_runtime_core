/*---
* Copyright (c) 2025 Huawei Device Co., Ltd.
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
---*/

/*---
desc: generic parameters with constraint in extends
---*/

class A {}
class B extends A{}
class C {}
class D<T extends A> {}
class E<T extends A | C> {}
class F<T extends Object> {
    data: T
    constructor(p: T) {
        this.data = p
        let o: Object = p
        p.toString()
    }
}

function main(): void {
    new D<A>();
    new D<B>();
    new E<A>();
    new E<B>();
    new E<C>();
    new F<A>(new A());
    new F<B>(new B());
    new F<C>(new C());
}

