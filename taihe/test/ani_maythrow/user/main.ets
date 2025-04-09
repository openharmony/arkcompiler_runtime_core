/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import {maythrow} from "@generated/maythrow";
import * as System from "std/core/System";

loadLibrary("ani_maythrow");

function check_no_error() {
    let res = maythrow(1)
    assertEQ(res, 11)
}

function check_error() {
    try {
        let res = maythrow(0)
    } catch (e) {
        assertEQ(e.toString(), "Error: some error happen")
    }
}

function main() {
    console.log("##############start#############")
    const suite = new ArkTestsuite("throw error ut")

    suite.addTest("check no error", check_no_error)
    suite.addTest("check error", check_error)
    System.exit(suite.run())
}
