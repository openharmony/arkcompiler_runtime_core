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

/*---
desc: Issue 14804. Smart casts implementation
tags: [compile-only]
---*/

function castValueToBoolean(value?: boolean) {
    if(value === undefined) return

    let innerValue = !value
}

function castValueToString(value: Object) {
    if(value instanceof String) {
        value += "Hello Smart Cast"
    }
}

interface Consumer {
    accept: (value: String) => undefined
}

class ConsumerImpl implements Consumer {
    accept: (value: String) => undefined = s =>  { console.log(s.length) }
}
