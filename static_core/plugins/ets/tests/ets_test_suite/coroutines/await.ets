/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

import {launch} from "std/concurrency";
import {Job} from "std/core";

let flag = false

function set_flag(): String {
    flag = true
    return "Paragliding is awesome!!\n"
}

export function main(): int {
    let p = launch<String, () => String>(set_flag);
    let result = p.Await();

    if (!flag) {
        console.print("Flag is not set!");
        return 1;
    } else {
        console.print("First await result: " + result);
        let result2 = p.Await();
        console.print("Second await result: " + result2);
        if (!(result === result2)) {
            console.print("await results do not match!");
            return 1;
        }
        if (!(result instanceof String)) {
            console.print("await result is not String!");
            return 1;
        }
    }
    return 0;
}
