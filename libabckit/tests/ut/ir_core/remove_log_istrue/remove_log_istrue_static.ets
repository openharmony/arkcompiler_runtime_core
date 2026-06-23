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

function erase<T>(v: T): Any {
    return v;
}

function checkErrorCode(code: int): int {
    let codeObj: Any = erase(new Int(code));
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('log before istrue');
    if (!!codeObj) {
        return 1;
    }
    return 0;
}

function main() {
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('done', checkErrorCode(42), checkErrorCode(0));
}
