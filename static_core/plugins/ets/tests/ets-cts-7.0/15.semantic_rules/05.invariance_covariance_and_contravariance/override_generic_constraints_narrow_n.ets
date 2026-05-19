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
desc: >-
    Override-compatible signatures require type parameter constraints to be contravariant (15.8.3 rule 6).
    Narrowing a constraint (Object -> String) violates this rule.
tags: [compile-only, negative]
expected_out: "Semantic error ESE150729: Class member foo with type T in WidenSubN cannot override foo with type T field from class WidenBaseN because the overriding type parameter's constraints are not compatible with type parameter constraints of the overridden method."
---*/

class WidenBaseN {
    foo<T extends Object>(p: T): T { return p }
}
class WidenSubN extends WidenBaseN {
    override foo<T extends String>(p: T): T { return p }
}

function main(): void {}
