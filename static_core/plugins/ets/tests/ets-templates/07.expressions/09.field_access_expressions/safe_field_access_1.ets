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
desc: Safe access to field
assert: A field access in which objectReference contains ‘?.’ is called safe field access because it handles null safely. The type of "a?." is never, because "?." removes null and undefined from the type, only never is remained.
tags: [negative, compile-only]
---*/

class A {
  public x: int;
}

function main(): void {
    let instance: A|null = null;
    assertTrue( instance?.x instanceof undefined )
}
