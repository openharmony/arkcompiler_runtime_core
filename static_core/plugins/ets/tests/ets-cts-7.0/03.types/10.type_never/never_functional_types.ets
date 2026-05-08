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
    functional types with never override
---*/

type F = () => void;

function main() {
    //never - bottom type for all types
    let f1: () => number = (): never => { throw new Error() }

    //with param
    let f2: (x: int) => string = (x: int): never => { throw new Error() }

    //via type
    let f3: F = (): never => { throw new Error() }
}
