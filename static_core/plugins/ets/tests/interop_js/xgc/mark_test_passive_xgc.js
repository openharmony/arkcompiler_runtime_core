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
const { init, triggerXGC, checkXRefsNumber } = require('./mark_test_utils.js');

let g_threasholdSize = 2048;
let g_array = new Array();
let g_etsVm;

// clear the cross-reference objects that are referenced by the active objects
function clearActiveRef() {
    g_array = new Array();
    g_etsVm.call('.clearActiveRef');
}

/**
 * Test xgc passive trigger
 * js obj <- sts pobj <- sts obj
 * js obj <- sts pobj <- sts obj <- root
 * js obj -> js pobj  -> sts obj
 * root   -> js obj   -> js pobj -> sts obj
 */
function passiveXGCTest() {
    let jsNum = 0;
    let stsNum = 0;
    let jsNumAfter = 0;
    let stsNumAfter = 0;
    checkXRefsNumber(jsNum, stsNum);
    for (let i = 0; i < g_threasholdSize; i++) {
        let j = i % 4;
        switch (j) {
            case 0:
                g_etsVm.call('.proxyJsObject', Promise.resolve(), false, false);
                jsNum++;
                break;
            case 1:
                g_etsVm.call('.proxyJsObject', Promise.resolve(), true, false);
                jsNum++;
                jsNumAfter++;
                break;
            case 2:
                g_etsVm.call('.createStsObject', false, false);
                stsNum++;
                break;
            default:
                g_array.push(g_etsVm.call('.createStsObject', false, false));
                stsNum++;
                stsNumAfter++;
        }
    }
    checkXRefsNumber(jsNum, stsNum);
    return {jsNumAfter: jsNumAfter, stsNumAfter: stsNumAfter};
}

g_etsVm = init('mark_test_passive_xgc_module', 'mark_test_sts.abc');

let res = passiveXGCTest();
// When the threshold is exceeded, the XGC is triggered
g_etsVm.call('.createStsObject', false, false);

checkXRefsNumber(res.jsNumAfter, res.stsNumAfter + 1);
clearActiveRef();
triggerXGC();
checkXRefsNumber(0, 0);
print('The passiveXGCTest method executes successful');
