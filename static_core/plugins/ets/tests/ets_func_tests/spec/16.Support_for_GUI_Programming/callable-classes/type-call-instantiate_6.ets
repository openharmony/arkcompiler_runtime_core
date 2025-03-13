/*
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

/*---
desc: 16.6 Callable Types
name: type-call-instantiate_6.ets
tags: []


---*/


class CallMeTrinity {
    name: String = "";

    static $_instantiate(factory: () => CallMeTrinity, newName: String): CallMeTrinity {
        let x = factory();
        x.name = newName;
        return x;
    }

    getName(): String {
        return this.name;
    }

}

function main(): int {
    let tt: CallMeTrinity = CallMeTrinity("Trinity");
    return tt.getName() == "Trinity" ? 0 : 1;
}
