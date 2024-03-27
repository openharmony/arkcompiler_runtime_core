/*---
Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
  The scope of a type parameter name in a function declaration is that entire declaration (function parameter scope).
tags:
- compile-only
- negative
---*/

let T = 1

function foo<T extends Number>(p: T): T {
    let x: T = p;
    let res: T = x * 2; // CTE: arithmetic is not applicable to type parameters
    return res;
}

function main(): void {
    assert foo<Number>(new Number(2.0)) == 4.0

    // module level scope is not affected
    assert T == 1
}
