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
desc: Issue 16167 Enum in conditional expression
tags: [compile-only]
---*/

function getFlag(): boolean {
    let cc = Math.random() > 0.5;
    return cc ? true : true;
}

let flag = getFlag();

enum OnPause {
    Nothing, Reset, Fade,
}

function f(input: OnPause): void {
    let op: OnPause = flag ? input : OnPause.Nothing;
}
