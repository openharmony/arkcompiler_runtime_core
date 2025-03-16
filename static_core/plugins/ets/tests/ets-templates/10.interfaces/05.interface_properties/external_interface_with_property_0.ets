/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: Declaring interface property
name: 10.interfaces/05.interface_properties/interface_property_declaration
files: ["./external_interface_with_property_1.ets"]
---*/

import {I, D} from "./external_interface_with_property_1.ets"

class C implements I {
    text: string = "";
    constructor() {
        this.text = "hello"
    }
}

export function main(): void {
    const c = new C();
    assertTrue(c.text == "hello");
    const d = new D<string>("hello");
    assertTrue(d.x == "hello");
}
