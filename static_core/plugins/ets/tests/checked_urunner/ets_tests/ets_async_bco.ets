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

//! CHECKER      Async bytecodes + ldai present with --opt-level=0 (no DCE)
//! RUN_FRONTEND options: "--opt-level=0 --stackless-coros=true --dump-assembly"
//! BC_METHOD    "ETSGLOBAL.foo"
//! INST         /ets\.async\.dispatch/
//! INST         /ets\.async\.await/
//! INST         /ets\.async\.unpack/
//! INST         /ets\.async\.resolve/
//! INST         /ets\.async\.reject/
//! INST         /ldai 0x1/
//! INST         /ldai 0x2/
//! INST         /ldai 0x3/

//! CHECKER      Async bytecodes survive --opt-level=2, ldai eliminated by DCE
//! RUN_FRONTEND options: "--stackless-coros=true --dump-assembly"
//! BC_METHOD    "ETSGLOBAL.foo"
//! INST         /ets\.async\.dispatch/
//! INST         /ets\.async\.await/
//! INST         /ets\.async\.unpack/
//! INST         /ets\.async\.resolve/
//! INST         /ets\.async\.reject/
//! INST_NOT     /ldai/

async function foo(x: Promise<int>) {
    let y = await x;
    let a = y + 1;
    a = y + 2;
    a = 3;
}
