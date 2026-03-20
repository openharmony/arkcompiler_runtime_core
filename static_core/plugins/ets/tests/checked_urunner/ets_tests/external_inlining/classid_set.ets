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
  files: [classid_set_b.ets, classid_set_c.ets]
---*/

// Source file must be last in the compile list
//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            inputs: "classid_set_b.ets", output: "classid_set_classid_set_b.ets.abc"
//! RUN_FRONTEND            inputs: "classid_set_c.ets", output: "classid_set_classid_set_c.ets.abc"
//! RUN_FRONTEND            options: "--opt-level=2 --ets-strings-concat=false"

import { Base, __noinline__createA as createA } from './classid_set_b.ets';
import { __noinline__createB as createB, __noinline__createC as createC } from './classid_set_c.ets';

//! CHECKER       Test classId monomorphic
//! SKIP_IF       @architecture == "arm32"
//! RUN_PGO_PROF  entry: "classid_set.ETSGLOBAL::main1", options: "--compiler-regex='.*main1.*' \
//!               --panda-files=../classid_set_classid_set_b.ets.abc:../classid_set_classid_set_c.ets.abc"
//! RUN_PGO_PAOC  options: "--compiler-regex='(.*process.*|.*main1.*)' \
//!               --panda-files=../classid_set_classid_set_b.ets.abc:../classid_set_classid_set_c.ets.abc"
//! METHOD        "classid_set.ETSGLOBAL::main1"
//! PASS_AFTER    "Inline"
//! INST          /DeoptimizeIf INLINE_IC/
//! RUN           entry: "classid_set.ETSGLOBAL::main1", options: "--log-debug=interop --panda-files=\
//!../classid_set.1.abc:../classid_set_classid_set_b.ets.abc:../classid_set_classid_set_c.ets.abc"
//! READ_FILE     "console.out"
//! INST_NOT      "D/interop: Destroy compiled method: classid_set.ETSGLOBAL::main1"

function process(obj: Base): int {
  return obj.getValue();
}

function main1(): int {
  let a: Base = createA(10);
  let sum = 0;
  for (let i = 0; i < 100; i++) {
    if (i % 2 === 0) {
      sum += process(a);
    }
  }
  return sum === 500 ? 0 : 1;
}

//! CHECKER       Test classId monomorphic two files
//! SKIP_IF       @architecture == "arm32"
//! RUN_PGO_PROF  entry: "classid_set.ETSGLOBAL::main3", options: "--compiler-regex='.*main3.*' \
//!               --panda-files=../classid_set_classid_set_b.ets.abc:../classid_set_classid_set_c.ets.abc"
//! RUN_PGO_PAOC  options: "--compiler-regex='(.*process.*|.*main3.*)' --log-debug=compiler --compiler-log=inlining \
//!               --panda-files=../classid_set_classid_set_b.ets.abc:../classid_set_classid_set_c.ets.abc",
//!               log_file: true
//! METHOD        "classid_set.ETSGLOBAL::main3"
//! PASS_AFTER    "Inline"
//! INST          /DeoptimizeIf INLINE_IC/
//! RUN           entry: "classid_set.ETSGLOBAL::main3", options: "--log-debug=interop --panda-files=\
//!../classid_set.1.abc:../classid_set_classid_set_b.ets.abc:../classid_set_classid_set_c.ets.abc"
//! READ_FILE     "console.out"
//! INST_NOT      "D/interop: Destroy compiled method: classid_set.ETSGLOBAL::main3"
//!
//! LOG_COMPILER    /Successfully inlined: classid_set_c.B::getValue/
//! LOG_COMPILER    /Successfully inlined: classid_set.ETSGLOBAL::process/
//! LOG_COMPILER    /successfully compiled method: classid_set.ETSGLOBAL::main3/

function main3(): int {
  let b: Base = createB(10);
  let sum = 0;
  for (let i = 0; i < 100; i++) {
    if (i % 2 === 0) {
      sum += process(b);
    }
  }
  return sum === 550 ? 0 : 1;
}

//! CHECKER       Test classId polymorphic
//! SKIP_IF       @architecture == "arm32"
//! RUN_PGO_PROF  entry: "classid_set.ETSGLOBAL::main2", options: "--compiler-regex='.*main2.*' \
//!               --panda-files=../classid_set_classid_set_b.ets.abc:../classid_set_classid_set_c.ets.abc"
//! RUN_PGO_PAOC  options: "--compiler-regex='(.*process.*|.*main2.*)' --log-debug=compiler --compiler-log=inlining \
//!               --panda-files=../classid_set_classid_set_b.ets.abc:../classid_set_classid_set_c.ets.abc",
//!               log_file: true
//! METHOD        "classid_set.ETSGLOBAL::main2"
//! PASS_AFTER    "Inline"
//! INST          /DeoptimizeIf INLINE_IC/
//! RUN           entry: "classid_set.ETSGLOBAL::main2", options: "--log-debug=interop --panda-files=\
//!../classid_set.1.abc:../classid_set_classid_set_b.ets.abc:../classid_set_classid_set_c.ets.abc"
//! READ_FILE     "console.out"
//! INST_NOT      "D/interop: Destroy compiled method: classid_set.ETSGLOBAL::main2"
//!
//! LOGS_COMPILER_COUNT    /Successfully inlined: classid_set_b.A::getValue/, 3
//! LOGS_COMPILER_COUNT    /Successfully inlined: classid_set_c.B::getValue/, 3
//! LOGS_COMPILER_COUNT    /Successfully inlined: classid_set.ETSGLOBAL::process/, 2

function main2(): int {
  let a: Base = createA(10);
  let b: Base = createB(20);
  let sum = 0;
  for (let i = 0; i < 100; i++) {
    if (i % 2 === 0) {
      sum += process(a);
    } else {
      sum += process(b);
    }
  }
  return sum === 1550 ? 0 : 1;
}

//! CHECKER       Test classId monomorphic unresolvable receiver
//! SKIP_IF       @architecture == "arm32"
//! RUN_PGO_PROF  entry: "classid_set.ETSGLOBAL::main4", options: "--compiler-regex='.*main4.*' \
//!               --panda-files=../classid_set_classid_set_b.ets.abc:../classid_set_classid_set_c.ets.abc"
//! RUN_PGO_PAOC  options: "--compiler-regex='(.*processC.*|.*main4.*)' --log-debug=compiler --compiler-log=inlining \
//!               --panda-files=../classid_set_classid_set_b.ets.abc:../classid_set_classid_set_c.ets.abc"
//! METHOD        "classid_set.ETSGLOBAL::main4"
//! PASS_AFTER    "Inline"
//! INST_NOT      /DeoptimizeIf INLINE_IC/
//! EVENT         /Inline,.*processC.*,.*getValue.*,.*,FAIL_RESOLVE/
//! RUN           entry: "classid_set.ETSGLOBAL::main4", options: "--panda-files=\
//!../classid_set.1.abc:../classid_set_classid_set_b.ets.abc:../classid_set_classid_set_c.ets.abc"

function processC(obj: Base): int {
  return obj.getValue();
}

function main4(): int {
  let c: Base = createC(10);
  let sum = 0;
  for (let i = 0; i < 100; i++) {
    sum += processC(c);
  }
  // C does not override getValue(), Base::getValue() returns 0
  return sum === 0 ? 0 : 1;
}

//! CHECKER       Test classId polymorphic with unresolvable receiver
//! SKIP_IF       @architecture == "arm32"
//! RUN_PGO_PROF  entry: "classid_set.ETSGLOBAL::main5", options: "--compiler-regex='.*main5.*' \
//!               --panda-files=../classid_set_classid_set_b.ets.abc:../classid_set_classid_set_c.ets.abc"
//! RUN_PGO_PAOC  options: "--compiler-regex='(.*processAC.*|.*main5.*)' --log-debug=compiler --compiler-log=inlining \
//!               --panda-files=../classid_set_classid_set_b.ets.abc:../classid_set_classid_set_c.ets.abc"
//! METHOD        "classid_set.ETSGLOBAL::main5"
//! PASS_AFTER    "Inline"
//! INST_NOT      /DeoptimizeIf INLINE_IC/
//! EVENT         /Inline,.*processAC.*,.*getValue.*,.*,FAIL_RESOLVE/
//! RUN           entry: "classid_set.ETSGLOBAL::main5", options: "--panda-files=\
//!../classid_set.1.abc:../classid_set_classid_set_b.ets.abc:../classid_set_classid_set_c.ets.abc"

function processAC(obj: Base): int {
  return obj.getValue();
}

function main5(): int {
  let a: Base = createA(10);
  let c: Base = createC(20);
  let sum = 0;
  for (let i = 0; i < 100; i++) {
    if (i % 2 === 0) {
      sum += processAC(a);
    } else {
      sum += processAC(c);
    }
  }
  // A::getValue() returns 10 x 50, C inherits Base::getValue() returns 0
  return sum === 500 ? 0 : 1;
}
