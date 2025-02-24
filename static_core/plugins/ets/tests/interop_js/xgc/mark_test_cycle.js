/**
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
const { init, triggerXGC, checkXRefsNumber, validationXGCResult } = require('./mark_test_utils.js');

let g_obj = Promise.resolve();
let g_inlineObj = Promise.resolve();
let g_etsVm;

function clearActiveRef() {
    g_obj = Promise.resolve();
    g_inlineObj = Promise.resolve();
    g_etsVm.call('.clearActiveRef');
}

function clearRefStorage() {
    clearActiveRef();
    triggerXGC();
    checkXRefsNumber(0, 0);
}

/**
 * Circular reference test
 * @brief If there is a root object in the ring or the root object refers to the object in the ring,
 *          the reference table is not reclaimed
 *       root1 -> js obj  <- sts pobj
 *                  |           ^
 *                  v           |
 *                root2        root4
 *                  |           ^
 *                  v           |
 *               js pobj -> sts obj <- root3
 * @param isRootRef1  The js root object refers to the object in the ring
 * @param isRootRef2  There is a js root object in the ring
 * @param isRootRef3  The sts root object refers to the object in the ring
 * @param isRootRef4  There is a sts root object in the ring
 */
function cycleTest(isRootRef1, isRootRef2, isRootRef3, isRootRef4) {
    let obj = Promise.resolve();
    if (isRootRef2) {
        obj.ref = g_inlineObj;
        g_inlineObj.ref = g_etsVm.call('.cycle', obj, isRootRef4, isRootRef3);
    } else {
        obj.ref = g_etsVm.call('.cycle', obj, isRootRef4, isRootRef3);
    }
    if (isRootRef1) {
        g_obj.ref = obj;
    }
}

g_etsVm = init('mark_test_cycle_module', 'mark_test_sts.abc');

cycleTest(false, false, false, false);
validationXGCResult(1, 1, 0, 0);

cycleTest(false, false, false, true);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

cycleTest(false, false, true, false);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

cycleTest(false, false, true, true);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

cycleTest(false, true, false, false);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

cycleTest(false, true, false, true);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

cycleTest(false, true, true, false);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

cycleTest(false, true, true, true);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

cycleTest(true, false, false, false);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

cycleTest(true, false, false, true);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

cycleTest(true, false, true, false);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

cycleTest(true, false, true, true);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

cycleTest(true, true, false, false);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

cycleTest(true, true, false, true);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

cycleTest(true, true, true, false);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

cycleTest(true, true, true, true);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

print('mark_test_cycle executes sucessful');
