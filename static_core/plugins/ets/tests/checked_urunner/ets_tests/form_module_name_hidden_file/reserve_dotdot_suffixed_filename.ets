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
arktsconfig: arktsconfig.json
files:
  - arktsconfig.json
  - entry/test/.a.ets
  - entry/test/..a.ets
  - entry/test/...a.ets
  - entry/test/a..ets
  - entry/test/a...ets
  - entry/test/a....ets
---*/

//! CHECKER      Preserve a...ets filename in module name
//! RUN_FRONTEND options: "--dump-assembly --gen-stdlib=false --stdlib=#{File.expand_path('plugins/ets/stdlib', (@options.source[%r{.*static_core}] || (ENV['ARKCOMPILER_RUNTIME_CORE_PATH'].to_s + '/static_core')))} --arktsconfig=#{File.expand_path('arktsconfig.json', File.dirname(@options.source))}", output: "reserve_dotdot_suffixed_filename.frontend.abc"
//! BC_METHOD    "reserve_dotdot_suffixed_filename.ETSGLOBAL.main"
//! INST         "initobj.short entry.test.a...A.<ctor>:void;"
//! INST_NOT     "initobj.short entry.test.a.A.<ctor>:void;"
//! INST_NOT     "initobj.short entry.test.a....A.<ctor>:void;"
//! INST         "call.acc.short entry.test.a...A.value:i32;, 0x0"
//! INST_NOT     "call.acc.short entry.test.a.A.value:i32;, 0x0"
//! INST_NOT     "call.acc.short entry.test.a....A.value:i32;, 0x0"

import { A } from './entry/test/a..'

function main(): void {
    let a = new A()
    arktest.assertEQ(a.value(), 1)
}
