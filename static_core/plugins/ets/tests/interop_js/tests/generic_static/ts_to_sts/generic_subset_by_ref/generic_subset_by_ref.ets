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

import {
    SubsetByRefStatic,
    genericSubsetByRefStaticCallFromTs,
    BaseObj,
} from "main_js"

function checkSubsetByRefStatic(): boolean {
    const res = SubsetByRefStatic.get(BaseObj);

    return res.a as int == BaseObj.a as int
}

function checkGenericSubsetByRefStaticCallFromTs(): boolean {
    const res = genericSubsetByRefStaticCallFromTs(BaseObj);

    return res.a as int == BaseObj.a as int
}
