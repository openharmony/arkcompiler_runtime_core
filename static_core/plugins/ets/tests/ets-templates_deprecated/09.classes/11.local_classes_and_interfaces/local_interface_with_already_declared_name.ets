/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: A compile-time error occurs if a local class or interface declaration has a name that is used to declare a new local class or interface (unless such new local class or interface is declared within a class or interface declaration).
tags: [compile-only, negative]
---*/

function bar(): void {
  let B: int = 1;

  interface B {
    x: int;
  }
}
