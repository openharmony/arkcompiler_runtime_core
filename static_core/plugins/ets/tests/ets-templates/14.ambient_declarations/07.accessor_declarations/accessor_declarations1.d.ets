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
desc: Ambient Accessor declarations.
tags: [compile-only, not-a-test]
---*/

// setter and getter
export declare let saved_age: int;
export declare get age(): int;
export declare set age(a: int);

// only getter
export declare let saved_age1: int;
export declare get age_getter(): int;

// only setter
export declare let saved_age2: int;
export declare set age_setter(a: int);
