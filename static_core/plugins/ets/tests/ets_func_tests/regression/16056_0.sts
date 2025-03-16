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
desc: Issue 16056. Lambda capture cast to float segfault
tags: [compile-only]
---*/

type float32 = float

export type EasingCurve = (t: float32) => float32

function vis(v0: float32, f: float32): EasingCurve {
    return (value: float32): float32 => ((v0 / (-4.2) * f) * (Math.exp((-4.2) * value * f) -1)) as float32
}
