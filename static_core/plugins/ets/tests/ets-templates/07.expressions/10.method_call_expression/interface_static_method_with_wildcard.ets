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
desc: Call instance method of a interface with wildcard.
assert: A compile-time error occurs if typeArguments is present and any of the type arguments are wildcards.
tags: [negative, compile-only]
---*/

class B {}

interface A {
    static bar<T extends B>(x: T): T {
        return x;
    }
}

function main(): void {
    let b: B = A.bar<out B>(new B())
}
