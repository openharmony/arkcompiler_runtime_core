/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

/* `hotInf` is inlined firstly even without profiling */
//! CHECKER      Check inlining sequence without profiling
//! RUN          entry: "inlining_order_3.ETSGLOBAL::main", 
//!              options: "--compiler-regex='.*ETSGLOBAL::.*' --compiler-profiling-threshold=100 \
//!                 --compiler-hotness-threshold=30 --log-level=debug --log-components=compiler \
//!                 --compiler-log=inlining --no-async-jit"
//!
//! EVENT        /Inline,inlining_order_3.ETSGLOBAL::testLoops,inlining_order_3.ETSGLOBAL::hotInf,.*,STATIC,SUCCESS/
//! EVENT_NEXT   /Inline,inlining_order_3.ETSGLOBAL::testLoops,inlining_order_3.ETSGLOBAL::cold,.*,STATIC,SUCCESS/
//! EVENT_NEXT   /Inline,inlining_order_3.ETSGLOBAL::testLoops,inlining_order_3.ETSGLOBAL::hot,.*,STATIC,SUCCESS/
//!# EVENT_NEXT  \
//!#        /Inline,inlining_order_3.ETSGLOBAL::testLoops,inlining_order_3.ETSGLOBAL::hotCatch,.*,STATIC,SUCCESS/
//!#        (Inlining in catch blocks is not supported)

//! CHECKER      Check inlining sequence with profiling
//! RUN          entry: "inlining_order_3.ETSGLOBAL::main",
//!              options: "--compiler-regex='.*ETSGLOBAL::.*' --compiler-profiling-threshold=0 \
//!                 --compiler-hotness-threshold=30 --log-level=debug --log-components=compiler \
//!                 --compiler-log=inlining --no-async-jit"
//!
//! EVENT        /Inline,inlining_order_3.ETSGLOBAL::testLoops,inlining_order_3.ETSGLOBAL::hotInf,.*,STATIC,SUCCESS/
//! EVENT_NEXT   /Inline,inlining_order_3.ETSGLOBAL::testLoops,inlining_order_3.ETSGLOBAL::hot,.*,STATIC,SUCCESS/
//! EVENT_NEXT   /Inline,inlining_order_3.ETSGLOBAL::testLoops,inlining_order_3.ETSGLOBAL::cold,.*,STATIC,SUCCESS/
//!# EVENT_NEXT  \
//!#        /Inline,inlining_order_3.ETSGLOBAL::testLoops,inlining_order_3.ETSGLOBAL::hotCatch,.*,STATIC,SUCCESS/
//!#        (Inlining in catch blocks is not supported)

//! CHECKER      Check inlining sequence with profiling (AOT)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: ""
//! EVENT        /Inline,inlining_order_3.ETSGLOBAL::testLoops,inlining_order_3.ETSGLOBAL::hotInf,.*,STATIC,SUCCESS/
//! EVENT_NEXT   /Inline,inlining_order_3.ETSGLOBAL::testLoops,inlining_order_3.ETSGLOBAL::hot,.*,STATIC,SUCCESS/
//!
//!# EVENT_NEXT  \
//!#        /Inline,inlining_order_3.ETSGLOBAL::testLoops,inlining_order_3.ETSGLOBAL::hotCatch,.*,STATIC,SUCCESS/
//!#        (Inlining in catch blocks is not supported)
//!
//! EVENT_NEXT   /Inline,inlining_order_3.ETSGLOBAL::testLoops,inlining_order_3.ETSGLOBAL::cold,.*,STATIC,SUCCESS/
//! RUN          entry: "inlining_order_3.ETSGLOBAL::main",
//!              options: "--log-level=debug --log-components=compiler --compiler-log=inlining"

function cold(): int {
 return 30; 
}

function hotInf(i:int):int {
 return -i; 
}
function hotCatch(i:int):int {
 return -i; 
}
function hot(i:int):int {
 return -i; 
}

function testLoops(): void {
    let N = cold();
    let arr: FixedArray<int> = new FixedArray<int>(N);

    for (let i:int = 0; i < N; i++) {
        arr[i] += hot(i);
        try {
            for (;;) {
                arr[i + N] = hotInf(i);
            }
        } catch (e) {
            arr[i] += hotCatch(i);
        }
    }
}

function main(): int {
    for (let i = 0; i < 3; i++) {
        testLoops();
    }
    return 0;
}
