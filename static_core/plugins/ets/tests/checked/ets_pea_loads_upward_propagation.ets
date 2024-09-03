/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

//! CHECKER       PEA_propagate_load_objects_upwards JIT
//! RUN           force_jit: true, entry: "ETSGLOBAL::test1"
//! METHOD        "ETSGLOBAL::test1"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 0
//! INST_COUNT    /LoadObject.*res/, 0

//! CHECKER       PEA_propagate_load_objects_upwards AOT
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*::test1.*'"
//! METHOD        "ETSGLOBAL::test1"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject.*res/, 0
//! INST_COUNT    /LoadObject.*res/, 0
//! RUN           entry: "ETSGLOBAL::test1"

//! CHECKER       PEA_propagate_load_objects_upwards_two_loads JIT
//! RUN           force_jit: true, entry: "ETSGLOBAL::test2"
//! METHOD        "ETSGLOBAL::test2"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 2
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 0
//! INST_COUNT    /LoadObject.*res/, 0

//! CHECKER       PEA_propagate_load_objects_upwards_two_loads AOT
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*test2.*'"
//! METHOD        "ETSGLOBAL::test2"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 2
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject.*res/, 0
//! INST_COUNT    /LoadObject.*res/, 0
//! RUN           entry: "ETSGLOBAL::test2"

//! CHECKER       PEA_propagate_load_objects_upwards_two_branches_loads JIT
//! RUN           force_jit: true, entry: "ETSGLOBAL::test3"
//! METHOD        "ETSGLOBAL::test3"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 6
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 3
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 4
//! INST_COUNT    /StoreObject.*res/, 0
//! INST_COUNT    /LoadObject.*res/, 0

//! CHECKER       PEA_propagate_load_objects_upwards_two_branches_loads AOT
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*::test3.*'"
//! METHOD        "ETSGLOBAL::test3"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 3
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject.*res/, 0
//! INST_COUNT    /LoadObject.*res/, 0
//! RUN           entry: "ETSGLOBAL::test3"

//! CHECKER       PEA_propagate_load_objects_upwards_many_inputs_phi JIT
//! RUN           force_jit: true, entry: "ETSGLOBAL::test4"
//! METHOD        "ETSGLOBAL::test4"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 6
//! INST_COUNT    /StoreObject.*res/, 6
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 0
//! INST_COUNT    /LoadObject.*res/, 0

//! CHECKER       PEA_propagate_load_objects_upwards_many_inputs_phi AOT
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*::test4.*'"
//! METHOD        "ETSGLOBAL::test4"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 4
//! INST_COUNT    /StoreObject.*res/, 6
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject.*res/, 0
//! INST_COUNT    /LoadObject.*res/, 0
//! RUN           entry: "ETSGLOBAL::test4"

/* partial elimination as the in-loop object will be materialized bc of SaveState */
//! CHECKER       PEA_propagate_load_objects_upwards_many_inputs_phi_with_loop JIT
//! RUN           force_jit: true, options: "--compiler-loop-unroll=false", entry: "ETSGLOBAL::test5"
//! METHOD        "ETSGLOBAL::test5"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 4
//! INST_COUNT    /StoreObject.*res/, 2
//! INST_COUNT    /LoadObject.*res/, 1

//! CHECKER       PEA_propagate_load_objects_upwards_many_inputs_phi_with_loop AOT
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*::test5.*' --compiler-loop-unroll=false"
//! METHOD        "ETSGLOBAL::test5"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject.*res/, 2
//! INST_COUNT    /LoadObject.*res/, 1
//! RUN           entry: "ETSGLOBAL::test5"

//! CHECKER       PEA_propagate_load_objects_upwards_iterator JIT
//! RUN           force_jit: true, entry: "ETSGLOBAL::test9"
//! METHOD        "ETSGLOBAL::test9"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 7
//! INST_COUNT    /StoreObject.*done/, 2
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*done/, 0

//! CHECKER       PEA_propagate_load_objects_upwards_neg_no_ld_users JIT
//! RUN           force_jit: true, options: "--compiler-inlining-blacklist=ETSGLOBAL::getRef", entry: "ETSGLOBAL::ntest1"
//! METHOD        "ETSGLOBAL::ntest1"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4

//! CHECKER       PEA_propagate_load_objects_upwards_neg_no_ld_users AOT
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-inlining-blacklist=ETSGLOBAL::getRef --compiler-regex='.*::ntest1.*'"
//! METHOD        "ETSGLOBAL::ntest1"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! RUN           entry: "ETSGLOBAL::ntest1"

//! CHECKER       PEA_propagate_load_objects_upwards_neg_mixed_users JIT
//! RUN           force_jit: true, options: "--compiler-inlining-blacklist=ETSGLOBAL::getRef", entry: "ETSGLOBAL::ntest2"
//! METHOD        "ETSGLOBAL::ntest2"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1

//! CHECKER       PEA_propagate_load_objects_upwards_neg_mixed_users AOT
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-inlining-blacklist=ETSGLOBAL::getRef --compiler-regex='.*::ntest2.*'"
//! METHOD        "ETSGLOBAL::ntest2"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//! RUN           entry: "ETSGLOBAL::ntest2"

//! CHECKER       PEA_propagate_load_objects_upwards_neg_diff_types JIT
//! RUN           force_jit: true, entry: "ETSGLOBAL::ntest3"
//! METHOD        "ETSGLOBAL::ntest3"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1

//! CHECKER       PEA_propagate_load_objects_upwards_neg_diff_types AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*::ntest3.*'"
//! METHOD        "ETSGLOBAL::ntest3"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//! RUN           entry: "ETSGLOBAL::ntest3"

//! CHECKER       PEA_propagate_load_objects_upwards_neg_many_inputs_phi_diff_types JIT
//! RUN           force_jit: true, entry: "ETSGLOBAL::ntest4"
//! METHOD        "ETSGLOBAL::ntest4"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 6
//! INST_COUNT    /StoreObject.*res/, 6
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 6
//! INST_COUNT    /StoreObject.*res/, 6
//! INST_COUNT    /LoadObject.*res/, 1

//! CHECKER       PEA_propagate_load_objects_upwards_neg_many_inputs_phi_diff_types AOT
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*::ntest4.*'"
//! METHOD        "ETSGLOBAL::ntest4"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 4
//! INST_COUNT    /StoreObject.*res/, 6
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 4
//! INST_COUNT    /StoreObject.*res/, 6
//! INST_COUNT    /LoadObject.*res/, 1
//! RUN           entry: "ETSGLOBAL::ntest4"



class BBB {
    public res: number;
    public res1: number;
    constructor() {
        this.res = 0;
        this.res1 = 0;
    }
    constructor(n: number) {
        this.res = n;
        this.res1 = n;
    }

}
class AAA extends BBB {
    constructor() { super()}
    constructor(n: number) { super(n)}
}

/* returns the same type objects */
function foo1(a: number): BBB {
    if (a > 0) {
        return new BBB(a);
    }
    return new BBB();
}

/* returns different types of objects */
function foo2(a: number): BBB {
    if (a > 0) {
        return new BBB(a);
    }
    return new AAA();
}

/* returns a sum calculated using iterators */
function foo3(a: Array<number>): number
{
    let res: number = 0;
    let values: IterableIterator<number> = a.values();
    for (let i: int = 0; i < a.length; ++i) {
        res += values.next().value as number;
    }
    return res;
}

/* simple one load case (one phi user) */
function test1(): int {
    let aa: Array<number> = new Array<number>
    aa.push(3);
    return foo1(aa[0]).res as int - 3;
}

/* two loads case (two phi users) */
function test2(): int {
    let aa: Array<number> = new Array<number>
    aa.push(3)
    let z: BBB = foo1(aa[0])
    return (z.res + z.res1) as int - 6;
}

/* more complex CFG: different number of loads in different branchs */
function test3(): int {
    let aa: Array<number> = new Array<number>
    aa.push(3)
    let t: boolean = (aa[0] > 0)
    let z: BBB = foo1(aa[0])
    if (t) {
        return z.res as int - 3
    } else {
        return (z.res + z.res1) as int - 6;
    }
}

/* test the stdlib iterator */
function test9(): int {
    let aa: Array<number> = new Array<number>
    aa.push(3);
    aa.push(5);
    aa.push(1);
    return foo3(aa) as int - 9;
}

/* many inputs phi */
function test4(): int {
    let aa: Array<number> = new Array<number>
    aa.push(3)
    let t: int = aa[0] as int;
    let a: BBB;
    if (t > 0) {
        if (t > 2) {
            a = new BBB(t + 1)
        } else {
            a = new BBB(t + 4)
        }
    } else {
        a = new BBB(t + 2)
    }
    if (a.res > 0) {
        return 0
    }
    return 1;
}

/* with phi with inputs comping from and beyond
 * the loop only a partial scalarization occurs */
function test5(): int {
    let aa: Array<number> = new Array<number>
    aa.push(3)
    let a: BBB = new BBB();
    let t: int = aa[0] as int;
    for (let i:int = 0; i < t; ++i) {
        a = new BBB(i)
        if (i % 2 == 0) {
            continue
        }
    }
    if (a.res > 0) {
        return 0
    }
    return 1;
}


// Negative tests

/* object consumer, not to be inlined */
function getRef(a: BBB): boolean {
    if (a.res == 3) {
        return true;
    }
    return false;
}

/* negative: the user is not a LoadObject */
function ntest1(): int {
    let aa: Array<number> = new Array<number>
    aa.push(3)
    let z: BBB = foo1(aa[0])
    if (getRef(z)) {
        return 0
    }
    return 1
}

/* negative: there is a user that is not a LoadObject */
function ntest2(): int {
    let aa: Array<number> = new Array<number>
    aa.push(3)
    let z: BBB = foo1(aa[0])
    let res: number = z.res
    if (getRef(z)) {
        return res as int - 3;
    }
    return 1
}

/* negative: the object types are different */
function ntest3(): int {
    let aa: Array<number> = new Array<number>
    aa.push(3);
    return foo2(aa[0]).res as int - 3;
}

/* negative: many inputs phi, different object types */
function ntest4(): int {
    let aa: Array<number> = new Array<number>
    aa.push(3)
    let t: int = aa[0] as int;
    let a: BBB;
    if (t > 0) {
        if (t > 2) {
            a = new BBB(t + 1)
        } else {
            a = new BBB(t + 4)
        }
    } else {
        a = new AAA(t + 2)
    }
    if (a.res > 0) {
        return 0
    }
    return 1;
}
