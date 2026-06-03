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
    Test narrowing reference conversion across packages.
    Covers conversion.cpp NarrowingReference, IsAllowedNarrowingReferenceConversion.
files: [lib_shapes.ets]
name: test_import
arktsconfig: arktsconfig-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "circle:r=5"
        - "square:s=4"
        - "draw:circle"
---*/

import { Shape, Circle, Square, Drawable, DrawableCircle } from '@panda/lib/lib_shapes'

let s1: Shape = new Circle(5)
let c = s1 as Circle
console.log(c.describe())

let s2: Shape = new Square(4)
let sq = s2 as Square
console.log(sq.describe())

let dc = new DrawableCircle(3)
let d: Drawable = dc
let dc2 = d as DrawableCircle
console.log(dc2.draw())
