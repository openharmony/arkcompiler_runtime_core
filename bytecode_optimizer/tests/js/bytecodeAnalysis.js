/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const a = 1;

// local export
export {a};
export {a as a1, b as b1, b as b2, a as a2};
const b = 2;

// regular import
import {i as i1, i5} from "@bundle:myapp/test";
import {i as i2, i6} from "@package:myapp/test";
import {i as i3, i7} from "@normalized:N&modulename&bundle&myapp/test&";
import {i as i4, i8} from "test";

// namespace import
import * as m4 from "@bundle:myapp/test";