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
    The export modifier is used by the build system to make a module
    accessible in other modules by import.
files:
- ./modules/mod_export.ets
---*/

import { Counter, add } from "./modules/mod_export"

function main(): int {
    let suite = new arktest.ArkTestsuite("Module Header Export")

    suite.addTest("Import class from exported module", () => {
        let c = new Counter()
        arktest.assertEQ(c.getCount(), 0)
        c.increment()
        c.increment()
        c.increment()
        arktest.assertEQ(c.getCount(), 3)
    })

    suite.addTest("Import function from exported module", () => {
        arktest.assertEQ(add(17, 19), 36)
    })

    return suite.run()
}
