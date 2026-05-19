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
desc: Wrong-direction subtyping for nested covariant generics. CovOuter<CovBox<Object>> is not a subtype of CovOuter<CovBox<String>>.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0318: Type 'CovOuter<CovBox<Object>>' cannot be assigned to type 'CovOuter<CovBox<String>>'"
---*/

class CovBox<out T> {
    readonly v: T
    constructor(v: T) { this.v = v }
}

class CovOuter<out U> {
    readonly b: U
    constructor(b: U) { this.b = b }
}

function main(): void {
    let o: CovOuter<CovBox<String>> = new CovOuter<CovBox<Object>>(new CovBox<Object>(new Object()))
}
