/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

function double2jsvalue(v: double): JSValue {
    return v;
}

function jsvalue2double(v: JSValue): double {
    return v;
}

function getPropertyFromJsvalue(obj: JSValue): double {
    return obj.prop;
}

function getPropertyFromJsvalue2(obj: JSValue): double {
    return obj.prop_1.prop_2;
}

function setPropertyToJsvalue(obj: JSValue, v: double): void {
    obj.prop = v;
}
