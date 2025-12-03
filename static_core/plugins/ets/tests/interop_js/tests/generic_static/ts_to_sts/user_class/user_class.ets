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

let module = ESObject.load('../../plugins/ets/tests/interop_js/tests/generic_static/ts_to_sts/main');
let ts_int = module.getProperty('ts_int');
let User = module.getProperty('User');
let UserClassStatic = module.getProperty('UserClassStatic');
let userClassFromTs = module.getProperty('userClassFromTs');
let checkInstance = module.getProperty('checkInstance');

function checkUserClassStatic(): boolean {
    return checkInstance.invoke(UserClassStatic.invokeMethod("get", User.instantiate(ts_int)), User).toBoolean();
}

function checkUserClassFromTs(): boolean {
    return checkInstance.invoke(userClassFromTs.invoke(User.instantiate(ts_int)), User).toBoolean();
}