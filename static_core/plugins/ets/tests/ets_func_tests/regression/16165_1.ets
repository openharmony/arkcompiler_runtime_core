/*
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

/*---
desc: Issue 16165 Union of Enum and undefined
tags: []
---*/

enum OnPause {
    Nothing, Reset, Fade,
}

function runState0(state?: OnPause) {
    assertEQ(undefined, state)
}

function runState1(state?: OnPause) {
    assertEQ(OnPause.Nothing, state)
}

function main() {
    let ue0: OnPause | undefined = undefined
    let ue1: OnPause | undefined = OnPause.Nothing

    runState0(ue0);
    runState1(ue1);


}
