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

let registry: FinalizationRegistry<int> | null = null;
let registry2: FinalizationRegistry<int> | null = null;


function doFullGC(): void {
    try {
        GC.waitForFinishGC(GC.startGC(GC.FULL_CAUSE, GC.IN_PLACE_MODE));
    } catch (e) {
        assertTrue(false, "Unexpected exception during GC");
    }
}

function fillFinReg(registry: FinalizationRegistry<int> | null) {
    let object1: Object | null =  new Object();
    let object1_callbackArg = 1;
    registry!.register(object1!, object1_callbackArg);
    object1 = null;
}

function createFinReg(callback: (value: int) => void) : FinalizationRegistry<int>  {
    return new FinalizationRegistry<int>(callback);
}

function InitFinRegAndFill(callback: (value: int) => void, isMainCoro: boolean = true) {
    if (isMainCoro) {
        registry = createFinReg(callback);
        fillFinReg(registry);
    } else {
        registry2 = createFinReg(callback);
        fillFinReg(registry2);
    }
}

function runGCInDiffThreads() {
    doFullGC();
    await launch doFullGC();
}

function testFinRegOnMainAndDiffCoro() {
    let result_finreg_main : int = 0;
    let result_finreg_not_main : int = 0;
    let p = await launch InitFinRegAndFill((value: int) => {
        result_finreg_not_main = value;
    }, false);
    InitFinRegAndFill((value: int) => {
        result_finreg_main = value;
    }, true);
    runGCInDiffThreads();
    while(!result_finreg_not_main) {}
    assertEQ(result_finreg_not_main, 1);
    // We need to switch from main coroutine to coroutine created by FinalizationRegistry because 
    // if main coro is executed first, the test will return false
    Coroutine.Schedule();
    assertEQ(result_finreg_main, 1);
}

function main(): void {
    testFinRegOnMainAndDiffCoro();
}