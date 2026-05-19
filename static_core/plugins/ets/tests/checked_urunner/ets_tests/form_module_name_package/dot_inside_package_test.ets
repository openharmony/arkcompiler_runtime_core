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
  - .pkg/pkg.ets
  - ..pkg/pkg.ets
  - ...pkg/pkg.ets
  - pkg./pkg.ets
  - pkg../pkg.ets
  - pkg.../pkg.ets
  - p.k.g/pkg.ets
---*/

//! CHECKER      Resolve dot_inside_package path alias to p.k.g directory
//! RUN_FRONTEND options: "--dump-assembly --gen-stdlib=false --stdlib=#{File.expand_path('plugins/ets/stdlib', (@options.source[%r{.*static_core}] || (ENV['ARKCOMPILER_RUNTIME_CORE_PATH'].to_s + '/static_core')))} --arktsconfig=#{File.expand_path('arktsconfig.json', File.dirname(@options.source))}", output: "dot_inside_package_test.frontend.abc"
//! BC_METHOD    "dot_inside_package_test.ETSGLOBAL.main"
//! INST         "initobj.short dot_inside_package.pkg.B.<ctor>:void;"
//! INST_NOT     "initobj.short pkg.B.<ctor>:void;"
//! INST_NOT     "initobj.short p.k.g.pkg.B.<ctor>:void;"
//! INST         "call.acc.short dot_inside_package.pkg.B.value:i32;, 0x0"
//! INST_NOT     "call.acc.short pkg.B.value:i32;, 0x0"
//! INST_NOT     "call.acc.short p.k.g.pkg.B.value:i32;, 0x0"

import { B } from 'dot_inside_package/pkg'

function main(): void {
    let a = new B()
    arktest.assertEQ(a.value(), 1)
}
