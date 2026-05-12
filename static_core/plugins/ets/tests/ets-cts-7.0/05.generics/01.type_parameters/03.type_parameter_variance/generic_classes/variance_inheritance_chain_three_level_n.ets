/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
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

/*---
desc: Three-level covariant chain. CovC<Object> is not a subtype of CovA<String>.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0318: Type 'CovC<Object>' cannot be assigned to type 'CovA<String>'"
---*/

class CovA<out T> {
    readonly v: T
    constructor(v: T) { this.v = v }
}
class CovB<out T> extends CovA<T> {
    readonly w: T
    constructor(v: T, w: T) { super(v); this.w = w }
}
class CovC<out T> extends CovB<T> {
    constructor(v: T, w: T) { super(v, w) }
}

function main(): void {
    let a: CovA<String> = new CovC<String>("x", "y")
    a = new CovC<Object>(new Object(), new Object())
}
