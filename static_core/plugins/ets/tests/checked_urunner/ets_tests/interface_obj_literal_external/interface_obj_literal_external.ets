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

//! CHECKER      interface.interface$MyNamespace$Data$ObjectLiteral is external
//! RUN_FRONTEND options: "--dump-assembly --gen-stdlib=false --stdlib=#{File.expand_path('plugins/ets/stdlib', (@options.source[%r{.*static_core}] || (ENV['ARKCOMPILER_RUNTIME_CORE_PATH'].to_s + '/static_core')))} --arktsconfig=#{File.expand_path('arktsconfig.json', File.dirname(@options.source))}", output: "interface_obj_literal_external.frontend.abc"
//! BC_METHOD    "interface._obj_literal_external.ETSGLOBAL.main"
//! INST         "interface.interface$MyNamespace$Data$ObjectLiteral"
//! INST_NOT     "interface._obj_literal_external.interface$MyNamespace$Data$ObjectLiteral"

import MyNamespace from './interface';

function main(): void {
    let data: MyNamespace.Data = {
        dynamicRange: MyNamespace.DecodingDynamicRange.SDR
    };
    let _ = data;
}
