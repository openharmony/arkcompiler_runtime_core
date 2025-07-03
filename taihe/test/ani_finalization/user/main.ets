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
import {BusinessError} from "@ohos.base";
import * as Test from "finalization_test";

loadLibrary("ani_finalization");

function doFullGC() {
    try {
        GC.waitForFinishGC(GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE));
    } catch (e) {
        assertTrue(false, "Unexpected exception during GC");
    }
}

function test_finalization() {
    let flag: int = 0;

    let foo: Test.Foo|null = Test.makeFoo();
    foo.addCallback(() => {
        flag = 42;
    })
    foo.addCallback(() => {
        console.log("Bye!");
    })
    foo = null;

    console.log("...")

    doFullGC()
    Coroutine.Schedule()

    assertEQ(flag, 42);
}

function main() {
    console.log("begin")

    const suite = new ArkTestsuite("FuncTest Test");

    suite.addTest("test finalization", test_finalization);

    exit(suite.run());
}
