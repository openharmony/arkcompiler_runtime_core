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
desc: Ambient accessor declaration is an ambient version of Accessor Declarations.
files: ['./accessor_declarations1.ets']
---*/

// Note test breaks if file names differ in accessor_declarations1.d and accessor_declarations1.ets
import { saved_age, age, saved_age1, age_getter, saved_age2, age_setter } from './accessor_declarations1.d'

// modeled after similar tests in 14.ambient_declarations/01.constant_or_variable_declarations
function main() {
    saved_age = 0
    arktest.assertEQ(age, 0)
    age = 1
    arktest.assertEQ(age, 1)
    saved_age1 = 0
    arktest.assertEQ(age_getter, 0)
    saved_age2 = 0
    age_setter = 1
    arktest.assertEQ(saved_age2, 1)
}
