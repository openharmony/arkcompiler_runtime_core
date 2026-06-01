/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
const { init, triggerXGC } = require('./mark_test_utils.js');

let gEtsVm;
let gWeakD;

function cancelWeakRootObject() {
    ArkTools.GC.clearWeakRefForTest();
}

function triggerJsFullGC() {
    ArkTools.GC.waitForFinishGC(ArkTools.GC.startGC('full'));
}

// Create D and C in a separate function so locals go out of scope.
// Pattern matches mark_test_gc.js createWeakRefObject1().
function createTestObjects() {
    const createCAndRefD = gEtsVm.getFunction('Lxgc_tests/ETSGLOBAL;', 'createCAndRefD');
    let objD = Promise.resolve();
    objD.data = new Array(10 * 1024 * 1024 / 8).fill(0);
    gWeakD = new WeakRef(objD);
    createCAndRefD(objD);
    // objD goes out of scope here — no JS root for D
    // objC proxy (return value) also goes out of scope — no JS root for C proxy
}

function stsRefJsWeakOnce(iteration) {
    const killC = gEtsVm.getFunction('Lxgc_tests/ETSGLOBAL;', 'killC');
    const verifyCCollected = gEtsVm.getFunction('Lxgc_tests/ETSGLOBAL;', 'verifyCCollected');

    createTestObjects();
    killC();

    cancelWeakRootObject();
    triggerXGC();
    triggerJsFullGC();

    if (gWeakD.deref()) {
        throw new Error('Iteration ' + iteration + ': JS D should be collected!');
    }

    verifyCCollected();
}

gEtsVm = init('mark_test_sts_ref_js_weak_module', 'xgc_tests.abc');

for (let i = 0; i < 100; i++) {
    stsRefJsWeakOnce(i);
}

print('mark_test_sts_ref_js_weak (100 iterations) executes successful');
