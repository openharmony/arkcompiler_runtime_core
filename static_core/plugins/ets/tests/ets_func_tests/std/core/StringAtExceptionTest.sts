/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

function main(): int {
    let failures = 0;

    let src: String = "All mimsy were the borgoves";
    try {
        src.at(100);
    } catch (e) {
        let b: boolean = e instanceof ArgumentOutOfRangeException;
        if(!b) {
            console.println("Unexpected exception" + e);
            return 1;
        }
    }
    src = "Twas brillig, and the slithy toves";
    try {
        src.at(-100);
    } catch (e) {
        let b: boolean = e instanceof ArgumentOutOfRangeException;
        if(!b) {
            console.println("Unexpected exception" + e);
            return 1;
        }
    }

    return 0;
}
