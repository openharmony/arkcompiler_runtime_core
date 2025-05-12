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

import {json} from "@ohos.util.json";

class Obj {
    a: int = 1 
    foo: string = "str"
    bar: Object = new Object()
    undef: undefined
}

function main() {
    testJsonHas()
}

function testJsonHas() {
    let o = new Obj()
    assertEQ(json.has(o, "a"), true)
    assertEQ(json.has(o, "bar"), true)
    assertEQ(json.has(o, "foo"), true)
    assertEQ(json.has(o, "blablabla"), false)
}