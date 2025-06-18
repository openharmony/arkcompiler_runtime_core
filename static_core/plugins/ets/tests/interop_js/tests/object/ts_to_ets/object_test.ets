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

let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/object/ts_to_ets/test_object')
let numb_valBoxed = module.getProperty('numb_valBoxed');
let bool_valBoxed = module.getProperty('bool_valBoxed');
let str_valBoxed = module.getProperty('str_valBoxed');
 
function ObjectTestValue(): boolean {
    if(numb_valBoxed.toNumber() !== 1) {
        return false;
    }
    return true;
}

function ObjectStringTestValue(): boolean {
    if(str_valBoxed.toString() !== 'eraycinar') {
        return false;
    }
    return true;
}

function ObjectBooleanTestValue(): boolean {
    if(bool_valBoxed.toBoolean() !== true) {
        return false;
    }
    return true;
}
