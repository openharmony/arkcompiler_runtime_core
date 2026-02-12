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

//! CHECKER      Inlining cache emits PUT then GET for reusable callee
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-enable-events --compiler-events-path=./events.csv \
//!                --compiler-regex='inlining_cache_events\.ETSGLOBAL::mainPositive'"
//! EVENT        /inlining_cache_events.*,operation:PUT,.*,inline_state:UNKNOWN/
//! EVENT_NEXT   /inlining_cache_events.*,operation:GET,.*,inline_state:OK/
//! EVENT        /Inline,inlining_cache_events.ETSGLOBAL::mainPositive,.*inlineReusable,.*,STATIC,SUCCESS/

//! CHECKER      Inlining cache reuses negative simple-only result
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-enable-events --compiler-events-path=./events.csv \
//!                --compiler-inline-simple-only=true \
//!                --compiler-regex='inlining_cache_events\.ETSGLOBAL::mainNegative'"
//! EVENT        /inlining_cache_events.*,operation:PUT,.*,inline_state:UNKNOWN/
//! EVENT_NEXT   /inlining_cache_events.*,operation:GET,.*,inline_state:FAIL/
//! EVENT        /Inline,inlining_cache_events.ETSGLOBAL::mainNegative,.*runtimeCallCallee,.*,STATIC,UNSUITABLE/

//! CHECKER      Inlining cache tracks external state in AOT PGO
//! SKIP_IF      @architecture == "arm32"
//! RUN_PGO_PROF entry: "inlining_cache_events.ETSGLOBAL::mainExternal"
//! RUN_PGO_PAOC options: "--compiler-enable-events --compiler-events-path=./events.csv \
//!                --compiler-inline-external-methods-aot=true \
//!                --compiler-regex='inlining_cache_events\.ETSGLOBAL::mainExternal'"
//! EVENT        /inlining_cache_events.*,operation:PUT,.*,external_state:UNKNOWN,inline_state:UNKNOWN/
//! EVENT_NEXT   /inlining_cache_events.*,operation:GET,.*,external_state:OK,inline_state:OK/
//! EVENT_NOT    /Inline,inlining_cache_events.ETSGLOBAL::mainExternal,.*/

//! CHECKER      Inlining cache events are AOT-only and absent in JIT
//! SKIP_IF      @architecture == "arm32"
//! RUN          force_jit: true, options: "--compiler-enable-events --compiler-events-path=./events.csv \
//!                --coroutine-workers-count=0 \
//!                --compiler-regex='inlining_cache_events\.ETSGLOBAL::mainPositive'", \
//!              entry: "inlining_cache_events.ETSGLOBAL::mainPositive"
//! EVENT_NOT    /inlining_cache_events.ETSGLOBAL::mainPositive,inlining-cache.*/
//! EVENT        /Inline,inlining_cache_events.ETSGLOBAL::mainPositive,.*inlineReusable,.*,STATIC,SUCCESS/

//! CHECKER      Inlining cache events are absent in JIT for negative case
//! SKIP_IF      @architecture == "arm32"
//! RUN          force_jit: true, options: "--compiler-enable-events --compiler-events-path=./events.csv \
//!                --coroutine-workers-count=0 \
//!                --compiler-inline-simple-only=true \
//!                --compiler-regex='inlining_cache_events\.ETSGLOBAL::mainNegative'", \
//!              entry: "inlining_cache_events.ETSGLOBAL::mainNegative"
//! EVENT_NOT    /inlining_cache_events.ETSGLOBAL::mainNegative,inlining-cache.*/
//! EVENT        /Inline,inlining_cache_events.ETSGLOBAL::mainNegative,.*runtimeCallCallee,.*,STATIC,UNSUITABLE/

//! CHECKER      Inlining cache events are absent in JIT for external case
//! SKIP_IF      @architecture == "arm32"
//! RUN          force_jit: true, options: "--compiler-enable-events --compiler-events-path=./events.csv \
//!                --coroutine-workers-count=0 \
//!                --compiler-inline-external-methods-aot=true \
//!                --compiler-regex='inlining_cache_events\.ETSGLOBAL::mainExternal'", \
//!              entry: "inlining_cache_events.ETSGLOBAL::mainExternal"
//! EVENT_NOT    /inlining_cache_events.ETSGLOBAL::mainExternal,inlining-cache.*/
//! EVENT_NOT    /Inline,inlining_cache_events.ETSGLOBAL::mainExternal,.*/

function inlineReusable(a0: int, a1: int): int {
    let v: int = 2;
    return a0 + a1 + v;
}

function runtimeCallCallee(a0: int): int {
    let v: int = 123;
    return v / a0;
}

function mainPositive(): int {
    let a0 = 3;
    let a1 = 4;
    let x = inlineReusable(a0, a1);
    inlineReusable(x, a1);
    return 0;
}

function mainNegative(): int {
    let x = 1;
    x = runtimeCallCallee(x);
    runtimeCallCallee(x);
    return 0;
}

function mainExternal(): int {
    let arr: Array<int> = [1, 2, 3, 4];
    arr.reverse();
    arr.reverse();
    return 0;
}
