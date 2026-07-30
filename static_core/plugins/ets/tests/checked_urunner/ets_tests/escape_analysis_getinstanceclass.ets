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

//! CHECKER       AOT PGO: EscapeAnalysis removes INLINE_IC GetInstanceClass
//! SKIP_IF       @architecture == "arm32"
//! RUN_PGO_PROF  entry: "escape_analysis_getinstanceclass.ETSGLOBAL::main"
//! RUN_PGO_PAOC  options: "--compiler-regex='escape_analysis_getinstanceclass\.ETSGLOBAL::test'"
//! METHOD        "escape_analysis_getinstanceclass.ETSGLOBAL::test"
//! PASS_AFTER    "Inline"
//! INST          /NewObject/
//! INST_NEXT     /CallVirtual.Inlined .*escape_analysis_getinstanceclass\.VirtualizeMe::toString/
//! INST_NEXT     /GetInstanceClass/
//! INST_NEXT     /DeoptimizeIf\s+INLINE_IC/
//! PASS_AFTER    "EscapeAnalysis"
//! INST          /LoadAndInitClass '.*VirtualizeMe'/
//! INST_NEXT     /CallVirtual.Inlined .*escape_analysis_getinstanceclass\.VirtualizeMe::toString/
//! INST_NOT      /GetInstanceClass/
//! INST_NEXT     /Deoptimize\s+INLINE_IC/
//! RUN           entry: "escape_analysis_getinstanceclass.ETSGLOBAL::main"
//! EVENT_NOT     /DeoptimizationReason,.*,INLINE_IC/

class VirtualizeMe {
    constructor() {}

    toString(): string {
        return 'test';
    }
}

let globalRef: Object | undefined = undefined
function setValue() {
    globalRef = new VirtualizeMe(); // allocate val here, virtualize after LSE
}
function getValue(): Object | undefined {
    const val = globalRef; // LSE replaces read with actual stored val
    globalRef = undefined; // LSE removes shadowed store in setValue
    return val;
}

function test(): string {
    setValue();              // Hide val creation from ObjectTypePropagation
    const val = getValue()!; // NullCheck only after shadowing store
    // Peephole optimization for inline guard GetInstanceClass of val is disabled for AOT-PGO.
    // It's replaced with LoadAndInitClass input directly by EscapeAnalysis instead
    return val.toString();   // materialize val here at Deoptimize INLINE_IC after GetInstanceClass
}

function main(): void {
    for (let i = 0; i < 30; i++) {
        arktest.assertEQ(test(), 'test');
    }
}
