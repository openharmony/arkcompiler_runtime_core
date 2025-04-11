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

class CheckCtorTypes {
    constructor(z: boolean, c: char, b: byte, s: short, i: int, l: long, f: float, d: double, r: string) {
        this.z = z
        this.c = c
        this.b = b
        this.s = s
        this.i = i
        this.l = l
        this.f = f
        this.d = d
        this.r = r
    }

    private z: boolean
    private c: char
    private b: byte
    private s: short
    private i: int
    private l: long
    private f: float
    private d: double
    private r: string
}
