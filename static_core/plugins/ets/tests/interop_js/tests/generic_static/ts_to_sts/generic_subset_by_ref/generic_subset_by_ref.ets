/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/generic_static/ts_to_sts/main');
let SubsetByRefStatic = module.getProperty('SubsetByRefStatic');
let genericSubsetByRefStaticCallFromTs = module.getProperty('genericSubsetByRefStaticCallFromTs');
let BaseObj = module.getProperty('BaseObj');

function checkSubsetByRefStatic(): boolean {
    try {
        const res = SubsetByRefStatic.invokeMethod('get', BaseObj);
        
        if (res == null) {
            return false;
        }
        
        const resA = res.getProperty('a');
        const baseObjA = BaseObj.getProperty('a');
        
        if (resA == null || baseObjA == null) {
            return false;
        }
        
        return resA.toNumber() === baseObjA.toNumber();
    } catch (e) {
        return false;
    }
}

function checkGenericSubsetByRefStaticCallFromTs(): boolean {
    try {
        const res = genericSubsetByRefStaticCallFromTs.invoke(BaseObj);
        
        if (res == null) {
            return false;
        }
        
        const resA = res.getProperty('a');
        const baseObjA = BaseObj.getProperty('a');
        
        if (resA == null || baseObjA == null) {
            return false;
        }
        
        return resA.toNumber() === baseObjA.toNumber();
    } catch (e) {
        return false;
    }
}
