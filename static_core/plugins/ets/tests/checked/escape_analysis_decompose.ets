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

//! CHECKER      JIT escape analysis decompose
//! SKIP_IF      @architecture == "arm32"
//! RUN          force_jit: true, options: "--compiler-regex=.*next[0-9]* --compiler-inlining=true", entry: "escape_analysis_decompose.ETSGLOBAL::main"
//!
//! EVENT        /DeoptimizationReason,escape_analysis_decompose.ETSGLOBAL::__noinline__next2,IFIMM_TRY/
//! EVENT        /Deoptimization,escape_analysis_decompose.ETSGLOBAL::__noinline__next2,4,IFRAME/
//! METHOD       "escape_analysis_decompose.ETSGLOBAL::__noinline__next2"
//! PASS_BEFORE  "EscapeAnalysis"
//! INST_COUNT   "SaveState", 8
//! INST_COUNT   "DeoptimizeIf", 1
//! PASS_AFTER   "EscapeAnalysis"
//! INST_COUNT   "SaveState", 9
//! INST_COUNT   "DeoptimizeIf", 1
//!
//! METHOD       "escape_analysis_decompose.ETSGLOBAL::__noinline__next3"
//! PASS_BEFORE  "EscapeAnalysis"
//! INST_COUNT   "SaveState", 5
//! INST_COUNT   "NullCheck", 1
//! PASS_AFTER   "EscapeAnalysis"
//! INST_COUNT   "SaveState", 5
//! INST_COUNT   "NullCheck", 1
//!
//! METHOD       "escape_analysis_decompose.ETSGLOBAL::next4"
//! PASS_BEFORE  "EscapeAnalysis"
//! INST_COUNT   "DeoptimizeIf", 1
//! INST_COUNT   "Deoptimize ", 1
//! PASS_AFTER   "EscapeAnalysis"
//! INST_COUNT   "DeoptimizeIf", 1
//! INST_COUNT   "Deoptimize ", 1

function __noinline__next1(str: string): string {
  return str;
}

// Positive test 1 - Decompose and compose normally
function __noinline__next2(str: string, arg: int) {
  try {
    if (arg > 0) {
      throw new Error(__noinline__next1(str) + 'ok');
    } else {
      return __noinline__next1(str) + 'ok';
    }
  } catch (e: Error) {
    return e.message;
  }
}

function doSmth() : void {
}

// Negative test 1 - NullCheck inst which has no flag CAN_DEOPTIMIZE
function __noinline__next3(str: String): String {
    let sb = new StringBuilder(str);
    sb.toString();
    doSmth();
    return sb.toString();
}

// Positive test 2 - Escape is applied and we removed some deopt instruction
class Int8ArrayIteratorEntries1 implements IterableIterator<[Number, Number]> {
    private parent: Int8Array
    private idx: int = 0

    constructor(parent: Int8Array) {
        this.parent = parent
    }

    public override $_iterator(): IterableIterator<[Number, Number]> {
        return this
    }

    override next(): IteratorResult<[Number, Number]> {
        if (this.idx < 0 || this.idx >= this.parent.length.toInt()) {
            return new IteratorResult<[Number, Number]>()
        }
        return new IteratorResult<[Number, Number]>(
            false, [new Number(this.idx), new Number(this.parent[this.idx++])]
        )
    }
}

// Positive test 3 - Deopt's savestate is in a matched inlined func
function next4(t : Int8ArrayIteratorEntries1) : IteratorResult<[Number, Number]> {
    return t.next()
}

function main(): void {
    for (let i = 0; i < 200; i++) {
        arktest.assertEQ(__noinline__next2('maybe', i < 100 ? 0 : 1), 'maybeok');
        arktest.assertEQ(__noinline__next3("abcde"), "abcde", "Wrong at next3")
    }
    let arr = new Int8Array([1, 2, 3]);
    let iter : Int8ArrayIteratorEntries1 = new Int8ArrayIteratorEntries1(arr)
    arktest.assertEQ(iter.next().done, false);
    arktest.assertEQ(next4(iter).done, false);
}
