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

/*---
desc: Declarations must be distinguishable in a declaration space.
---*/

// functions distinguishable by signatures

function foo(): int {
    return 1;
}

function foo(x: int): int {
    return 2;
}

function foo(x: int[]): int {
    return 3;
}

function foo(x: string): int {
    return 4;
}

function main(): int {
    if (foo() != 1) return 1;

    if (foo(1) != 2) return 1;

    if (foo(new int[0]) != 3) return 1;

    if (foo("") != 4) return 1;

    return 0;
}
