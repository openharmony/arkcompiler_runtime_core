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
desc: Switch statement
---*/

function f(i: int): void {
  switch (i) {
    case 0: assertEQ( i, 0); break;
    case 1: assertEQ( i, 1); break;
    case 2: assertEQ( i, 2); break;
    default: assertTrue( false );
  }
}

function main(): void {
  f(0);
  f(1);
  f(2);
}
