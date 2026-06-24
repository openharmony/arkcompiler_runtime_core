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
    Consumer importing anything from @panda/lib/lib_alias
    forces the consumer to parse `lib_alias` serialization, where the utility-type
    aliases are stored and that the parser should consume.
files: [lib_alias.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "default,12,y"
        - "light,12,y"
        - "dark,12,y"
---*/

import { Settings, PartialSettings, ReadonlySettings } from '@panda/lib/lib_alias'

function describePartial(p: PartialSettings): string {
    let theme: string = p.theme !== undefined ? p.theme! : "default"
    let fontSize: int = p.fontSize !== undefined ? p.fontSize! : 0
    let autoSave: string = p.autoSave !== undefined ? (p.autoSave! ? "y" : "n") : "n"
    return theme + "," + fontSize + "," + autoSave
}

function describeReadonly(r: ReadonlySettings): string {
    return r.theme + "," + r.fontSize + "," + (r.autoSave ? "y" : "n")
}

console.log(describePartial({ fontSize: 12, autoSave: true }))
console.log(describePartial({ theme: "light", fontSize: 12, autoSave: true }))
console.log(describeReadonly(new Settings()))
