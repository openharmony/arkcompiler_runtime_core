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
    Test annotation with array and enum property values.
files: [lib_meta.ets]
name: test_import
arktsconfig: arktsconfig-import.json
tags: [compile-only]
---*/

import { Severity, Logger } from '@panda/lib/lib_meta'

@Retention({policy: 'RUNTIME'})
@interface Config {
    severity: Severity = Severity.LOW
}

@Retention({policy: 'RUNTIME'})
@interface Tags {
    names: string[] = ["default"]
}

@Retention({policy: 'RUNTIME'})
@interface Levels {
    items: Severity[] = [Severity.LOW]
}

@Config({severity: Severity.HIGH})
class AlertService {
    notify(): void { Logger.log("alert") }
}

@Tags({names: ["web", "api"]})
class WebHandler {
    process(): void { Logger.log("web") }
}

@Levels({items: [Severity.HIGH, Severity.LOW, Severity.MEDIUM]})
class PriorityHandler {
    handle(): void { Logger.log("priority") }
}

function main(): void {
    let a = new AlertService()
    let w = new WebHandler()
    let p = new PriorityHandler()
}
