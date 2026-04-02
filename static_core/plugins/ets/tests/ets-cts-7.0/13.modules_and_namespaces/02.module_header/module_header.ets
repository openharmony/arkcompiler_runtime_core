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
    Module header defines optional export modifier and a module name.
    A module with a header can export declarations that are importable
    by other modules.
files:
- ./modules/mod_basic.ets
---*/

import * as M from "./modules/mod_basic"

function main(): int {
    let suite = new arktest.ArkTestsuite("Module Header")

    suite.addTest("Import function from module with header", () => {
        arktest.assertEQ(M.getValue(), 35)
    })

    suite.addTest("Import variable from module with header", () => {
        arktest.assertEQ(M.moduleVar, "hello")
    })

    return suite.run()
}
