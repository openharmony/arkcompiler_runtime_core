/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: >-
    Exported class with a tuple-typed field plus a tuple-returning
    method. The consumer reads, writes and replaces the field.
tags: [not-a-test, compile-only]
name: lib_coord
arktsconfig: arktsconfig-lib.json
es2panda_options: ['emit-declaration']
---*/

export class Coord {
    pos: [int, int] = [0, 0]
    label: string = ""
    constructor(x: int, y: int, l: string) {
        this.pos = [x, y]
        this.label = l
    }
    move(dx: int, dy: int): [int, int] {
        this.pos = [this.pos[0] + dx, this.pos[1] + dy]
        return this.pos
    }
    raw(): [int, int, string] {
        return [this.pos[0], this.pos[1], this.label]
    }
}
