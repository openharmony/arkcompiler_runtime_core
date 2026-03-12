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

//! CHECKER      Check LoadObject not reordered before corresponding CheckCast
//! RUN          force_jit: true,
//!              options: "--compiler-regex='.*doLoad.*'",
//!              entry: "checkcast_reordering.ETSGLOBAL::main"
//! METHOD       "checkcast_reordering.ETSGLOBAL::doLoad"
//! PASS_AFTER   "Scheduler"
//! INST         "NullCheck"
//! INST         "CheckCast"
//! INST_NEXT    /LoadObject.*A\.value/

//! CHECKER      Check LoadObject not reordered before corresponding CheckCast (AOT)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex='.*doLoad.*'"
//! METHOD       "checkcast_reordering.ETSGLOBAL::doLoad"
//! PASS_AFTER   "Scheduler"
//! INST         "NullCheck"
//! INST         "CheckCast"
//! INST_NEXT    /LoadObject.*A\.value/

class A {
    value: int

    constructor(value: int) {
        this.value = value
    }

    override toString(): string {
        return "A";
    }
}

function doLoad(a: Object) {
    return a.toString() + (a as A).value;
}

function main(): void {
    arktest.assertEQ(doLoad(new A(10)), "A10");
}
