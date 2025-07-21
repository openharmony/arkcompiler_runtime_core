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

//! CHECKER       AOT, check ets cache used by default
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--paoc-mode=aot --load-runtimes=ets --paoc-output=test.an"
//! RUN           options: "--compiler-enable-events --compiler-events-path=./events.csv --compiler-enable-jit=false --aot-file=test.an", entry: "ets_string_cache_options.ETSGLOBAL::main"
//!
//! EVENT         "EtsCache,Fastpath: create string from number with cache"

//! CHECKER       AOT, check ets cache not used with option
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--paoc-mode=aot --load-runtimes=ets --paoc-output=test.an"
//! RUN           options: "--compiler-enable-events --use-string-caches=false --compiler-events-path=./events.csv --compiler-enable-jit=false --aot-file=test.an", entry: "ets_string_cache_options.ETSGLOBAL::main"
//!
//! EVENT         "EtsCache,Slowpath: create string from number without cache"


//! CHECKER       JIT, check ets cache used by default
//! RUN           force_jit: true, options: "--compiler-enable-events --compiler-events-path=./events.csv", entry: "ets_string_cache_options.ETSGLOBAL::main"
//!
//! EVENT         "EtsCache,Fastpath: create string from number with cache"

//! CHECKER       JIT, check ets cache not used with option
//! RUN           force_jit: true, options: "--compiler-enable-events --use-string-caches=false --compiler-events-path=./events.csv", entry: "ets_string_cache_options.ETSGLOBAL::main"
//!
//! EVENT         "EtsCache,Slowpath: create string from number without cache"


//! CHECKER       INTERP-CPP, check ets cache used by default
//! RUN           options: "--interpreter-type=cpp --compiler-enable-events --compiler-events-path=./events.csv --compiler-enable-jit=false", entry: "ets_string_cache_options.ETSGLOBAL::main"
//!
//! EVENT         "EtsCache,Fastpath: create string from number with cache"

//! CHECKER       INTERP-CPP, check ets cache not used with option
//! RUN           options: "--interpreter-type=cpp --compiler-enable-events --use-string-caches=false --compiler-events-path=./events.csv --compiler-enable-jit=false", entry: "ets_string_cache_options.ETSGLOBAL::main"
//!
//! EVENT         "EtsCache,Slowpath: create string from number without cache"


//! CHECKER       INTERP-IRTOC, check ets cache used by default
//! RUN           options: "--interpreter-type=irtoc --compiler-enable-events --compiler-events-path=./events.csv --compiler-enable-jit=false", entry: "ets_string_cache_options.ETSGLOBAL::main"
//!
//! EVENT         "EtsCache,Fastpath: create string from number with cache"

//! CHECKER       INTERP-IRTOC, check ets cache not used with option
//! RUN           options: "--interpreter-type=irtoc --compiler-enable-events --use-string-caches=false --compiler-events-path=./events.csv --compiler-enable-jit=false", entry: "ets_string_cache_options.ETSGLOBAL::main"
//!
//! EVENT         "EtsCache,Slowpath: create string from number without cache"

function main() : void {
    assertEQ((123 as Long).toString(), "123")
}
