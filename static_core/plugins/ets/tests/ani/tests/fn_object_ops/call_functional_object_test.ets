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

class NotFnObj {
    invoke0(): String {
        return "test";
    }
}

function GetNonFnObj(): NotFnObj {
    return new NotFnObj();
}

function GetFnObj(): Object {
    let x = (a : String) => { return a; }
    return x;
}

function GetFnObjWithClosure(): Object {
    let y = "test";
    let x = (a : String) => { return y + a; }
    return x;
}
