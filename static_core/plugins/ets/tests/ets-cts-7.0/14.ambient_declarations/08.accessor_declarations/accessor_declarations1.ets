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
desc: Ambient Accessor declarations implementations.
tags: [compile-only, not-a-test]
---*/

// setter and getter
export let saved_age: int = 0
export get age(): int { return saved_age }
export set age(a: int) {
  saved_age = a
}

// only getter
export let saved_age1: int = 0
export get age_getter(): int { return saved_age1 }

// only setter
export let saved_age2: int = 0
export set age_setter(a: int) {
    saved_age2 = a
}
