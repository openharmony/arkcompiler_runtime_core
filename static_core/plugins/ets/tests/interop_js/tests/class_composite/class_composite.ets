/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

import { A } from "missing_js"

function Test1(): int {
    let a: A = { name: "Edalyn", age: 30, location: "Boiling Isles" };
    if (a.name as String != "Edalyn") {
        return 1;
    }
    if (a.age as int != 30) {
        return 2;
    }
    if (a.location as String != "Boiling Isles") {
        return 3;
    }
    return 0;
}

function Test2(): int {
    let a: A = { name: "Luz", age: 13, location: "Boiling Isles" };
    a.name = "king";
    if (a.name as String != "king") {
        return 1;
    }
    a.age = 5;
    if (a.age as int != 5) {
        return 2;
    }
    if (a.location as String != "Boiling Isles") {
        return 3;
    }
    return 0;
}
