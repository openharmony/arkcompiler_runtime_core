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
desc: Variance position violation. Out-parameter U appears in in-position when used as type argument of contravariant Inner<in T>.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0333: Type Parameter 'U' is declared as 'out' but occurs in 'in' position."
---*/

class ContraInner<in T> {
    accept(p: T): void {}
}

class CovMiddle<out U> {
    readonly inner: ContraInner<U>
    constructor(inner: ContraInner<U>) { this.inner = inner }
}

function main(): void {
}
