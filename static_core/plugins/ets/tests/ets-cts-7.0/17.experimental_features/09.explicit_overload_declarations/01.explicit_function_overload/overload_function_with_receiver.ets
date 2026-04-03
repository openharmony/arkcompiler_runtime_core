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
desc: An explicit overload can be called like a regular function
---*/

function bar1(this: string): string { return "1" }

function bar2(this: number): string { return "2" }

overload bar { bar1, bar2 }

let s: string = "1";
let n: number = 1;

function main(): void {
    arktest.assertEQ(bar(s), "1", "bar1 call expected");
    arktest.assertEQ(bar(n), "2", "bar2 call expected");
}
