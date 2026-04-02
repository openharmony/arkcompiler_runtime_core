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
desc: >-
    If a module header has the declare modifier then the whole module is ambient.
    Import from ambient module declaration backed by implementation.
files:
- ./modules/mod_declare.ets
---*/

import { foo, x, Bar } from "./modules/mod_declare.d"

function main(): int {
    let suite = new arktest.ArkTestsuite("Module Header Declare")

    suite.addTest("Import function from ambient module", () => {
        arktest.assertEQ(foo(), 37)
    })

    suite.addTest("Import variable from ambient module", () => {
        arktest.assertTrue(x > 3.0)
    })

    suite.addTest("Import class from ambient module", () => {
        let b = new Bar()
        arktest.assertEQ(b.getValue(), 39)
    })

    return suite.run()
}
