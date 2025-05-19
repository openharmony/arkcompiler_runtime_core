/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

//! CHECKER      Test check static launch inst in JIT
//! RUN          force_jit: true, options: "", entry: "launch_test.ETSGLOBAL::main"
//! METHOD       "launch_test.ETSGLOBAL::main"
//! PASS_AFTER   "IrBuilder"
//! INST         "LoadRuntimeClass"
//! INST_NEXT    "NewObject"
//! INST_NEXT    "CallLaunchStatic"
//! EVENT        /Compilation,launch_test.ETSGLOBAL::main,.*,COMPILED/

//! CHECKER      Test check unresolve static launch inst in JIT
//! RUN          force_jit: true, options: "--compiler-force-unresolved=true", entry: "launch_test.ETSGLOBAL::main"
//! METHOD       "launch_test.ETSGLOBAL::main"
//! PASS_AFTER   "IrBuilder"
//! INST         "LoadRuntimeClass"
//! INST_NEXT    "NewObject"
//! INST_NEXT    "ResolveStatic"
//! INST_NEXT    "CallResolvedLaunchStatic"
//! EVENT        /Compilation,launch_test.ETSGLOBAL::main,.*,COMPILED/

//! CHECKER      Test check static launch inst in AOT
//! RUN_PAOC     options: ""
//! METHOD       "launch_test.ETSGLOBAL::main"
//! PASS_AFTER   "IrBuilder"
//! INST         "LoadRuntimeClass"
//! INST_NEXT    "NewObject"
//! INST_NEXT    "ResolveStatic"
//! INST_NEXT    "CallResolvedLaunchStatic"
//! EVENT        /Compilation,launch_test.ETSGLOBAL::main,.*,COMPILED/
//! RUN          entry: "launch_test.ETSGLOBAL::main"

//! CHECKER      Test check static launch inst in LLVM
//! RUN_LLVM     options: ""
//! METHOD       "launch_test.ETSGLOBAL::main"
//! PASS_AFTER   "IrBuilder"
//! INST         "LoadRuntimeClass"
//! INST_NEXT    "NewObject"
//! INST_NEXT    "ResolveStatic"
//! INST_NEXT    "CallResolvedLaunchStatic"
//! RUN          entry: "launch_test.ETSGLOBAL::main"

//! CHECKER      Test check virtual launch inst in JIT
//! RUN          force_jit: true, options: "", entry: "launch_test.ETSGLOBAL::main1"
//! METHOD       "launch_test.ETSGLOBAL::main1"
//! PASS_AFTER   "IrBuilder"
//! INST         "LoadRuntimeClass"
//! INST_NEXT    "NewObject"
//! INST_NEXT    "CallLaunchVirtual"
//! EVENT        /Compilation,launch_test.ETSGLOBAL::main1,.*,COMPILED/

//! CHECKER      Test check virtual launch inst in AOT
//! RUN_PAOC     options: ""
//! METHOD       "launch_test.ETSGLOBAL::main1"
//! PASS_AFTER   "IrBuilder"
//! INST         "LoadRuntimeClass"
//! INST_NEXT    "NewObject"
//! INST_NEXT    "CallLaunchVirtual"
//! INST_NOT     "ResolveVirtual"
//! EVENT        /Compilation,launch_test.ETSGLOBAL::main1,.*,COMPILED/
//! RUN          entry: "launch_test.ETSGLOBAL::main1"

//! CHECKER      Test check virtual launch inst in LLVM
//! RUN_LLVM     options: ""
//! METHOD       "launch_test.ETSGLOBAL::main1"
//! PASS_AFTER   "IrBuilder"
//! INST         "LoadRuntimeClass"
//! INST_NEXT    "NewObject"
//! INST_NEXT    "CallLaunchVirtual"
//! INST_NOT     "ResolveVirtual"
//! RUN          entry: "launch_test.ETSGLOBAL::main1"

//! CHECKER      Test check unresolve virtual launch inst in JIT
//! RUN          force_jit: true, options: "--compiler-force-unresolved=true", entry: "launch_test.ETSGLOBAL::main1"
//! METHOD       "launch_test.ETSGLOBAL::main1"
//! PASS_AFTER   "IrBuilder"
//! INST         "LoadRuntimeClass"
//! INST_NEXT    "NewObject"
//! INST_NEXT    "ResolveVirtual"
//! INST_NEXT    "CallResolvedLaunchVirtual"
//! EVENT        /Compilation,launch_test.ETSGLOBAL::main1,.*,COMPILED/

//! CHECKER      Test check unresolve virtual launch inst in AOT
//! RUN_PAOC     options: "--compiler-force-unresolved=true"
//! METHOD       "launch_test.ETSGLOBAL::main1"
//! PASS_AFTER   "IrBuilder"
//! INST         "LoadRuntimeClass"
//! INST_NEXT    "NewObject"
//! INST_NEXT    "ResolveVirtual"
//! INST_NEXT    "CallResolvedLaunchVirtual"
//! EVENT        /Compilation,launch_test.ETSGLOBAL::main1,.*,COMPILED/
//! RUN          entry: "launch_test.ETSGLOBAL::main1"

//! CHECKER      Test check unresolve virtual launch inst in LLVM
//! RUN_LLVM     options: "--compiler-force-unresolved=true"
//! METHOD       "launch_test.ETSGLOBAL::main1"
//! PASS_AFTER   "IrBuilder"
//! INST         "LoadRuntimeClass"
//! INST_NEXT    "NewObject"
//! INST_NEXT    "ResolveVirtual"
//! INST_NEXT    "CallResolvedLaunchVirtual"
//! RUN          entry: "launch_test.ETSGLOBAL::main1"

//! CHECKER      Test check static launch inst in JIT with 2 arguments
//! RUN          force_jit: true, options: "", entry: "launch_test.ETSGLOBAL::main2"
//! METHOD       "launch_test.ETSGLOBAL::main2"
//! PASS_AFTER   "IrBuilder"
//! INST         "LoadRuntimeClass"
//! INST_NEXT    "NewObject"
//! INST_NEXT    "CallLaunchStatic"
//! EVENT        /Compilation,launch_test.ETSGLOBAL::main2,.*,COMPILED/

//! CHECKER      Test inline method with launch inst in JIT
//! RUN          force_jit: true, options: "", entry: "launch_test.ETSGLOBAL::main3"
//! METHOD       "launch_test.ETSGLOBAL::main3"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "LoadRuntimeClass"
//! INST_NOT     "ResolveVirtual"
//! INST_NOT     "CallResolvedLaunchVirtual"
//! PASS_AFTER   "Inline"
//! INST         "LoadRuntimeClass"
//! INST_NEXT    "NewObject"
//! INST_NEXT    "ResolveVirtual"
//! INST_NEXT    "CallResolvedLaunchVirtual"
//! EVENT        /Compilation,launch_test.ETSGLOBAL::main3,.*,COMPILED/
//! EVENT        /Inline,launch_test.ETSGLOBAL::main3,launch_test.ETSGLOBAL::launchF,.*STATIC,SUCCESS/

//! CHECKER      Test inline method with launch inst in AOT
//! RUN_PAOC     options: ""
//! METHOD       "launch_test.ETSGLOBAL::main3"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "LoadRuntimeClass"
//! INST_NOT     "ResolveVirtual"
//! INST_NOT     "CallResolvedLaunchVirtual"
//! PASS_AFTER   "Inline"
//! INST         "LoadRuntimeClass"
//! INST_NEXT    "NewObject"
//! INST_NEXT    "ResolveVirtual"
//! INST_NEXT    "CallResolvedLaunchVirtual"
//! EVENT        /Compilation,launch_test.ETSGLOBAL::main3,.*,COMPILED/
//! EVENT        /Inline,launch_test.ETSGLOBAL::main3,launch_test.ETSGLOBAL::launchF,.*STATIC,SUCCESS/
//! RUN          entry: "launch_test.ETSGLOBAL::main3"

//! CHECKER      Test inline method with launch inst in LLVM
//! RUN_LLVM     options: ""
//! METHOD       "launch_test.ETSGLOBAL::main3"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "LoadRuntimeClass"
//! INST_NOT     "ResolveVirtual"
//! INST_NOT     "CallResolvedLaunchVirtual"
//! RUN          entry: "launch_test.ETSGLOBAL::main3"

let counter = 0

class Foo {
    public Bar(a: int): String {
        counter -= a;
        return "Some dummy string"
    }
}

class Foo2 extends Foo {
    public override Bar(a: int): String {
        counter += a + a;
        return "Some dummy string"
    }
}


/* konstanting (#IAD5MH):
 * currently we have to return an object from any coroutine. This will change
 * once I add autoboxing for the primitive return values
 */
function add(a: int): String {
    counter += a;
    return "Some dummy string"
}

function add2(a1: int, a2: int): String {
    counter += a1 + a2;
    return "Some dummy string"
}

export function main(): int {
    launch add(5);
    Coroutine.Schedule();
    if (counter != 5) {
        console.print("Invalid counter: expected 5 but was " + counter + "\n");
        return 1;
    }
    return 0;
}

export function main1(): int {
    let obj: Foo = new Foo2();
    launch obj.Bar(3);
    Coroutine.Schedule();
    if (counter != 6) {
        console.print("Invalid counter: expected 6 but was " + counter + "\n");
        return 1;
    }
    return 0;
}

export function main2(): int {
    launch add2(3, 8);
    Coroutine.Schedule();
    if (counter != 11) {
        console.print("Invalid counter: expected 11 but was " + counter + "\n");
        return 1;
    }
    return 0;
}

function foo(arg1: int, arg2: int): int
{
    return arg1 + arg2;
}

function launchF(func: (p1: Int, p2: Int) => NullishType, arg1: Int, arg2: Int): Promise<NullishType>
{
    return launch func(arg1, arg2);
}

function main3(): int {
    let res = await launchF(foo, 1, 2);
    assertEQ(res, 3)
    return 0;
}