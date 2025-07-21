/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
//! RUN           force_jit: true, options: "--compiler-regex='.*test.*'", entry: "ets_pea_loads_upward_propagation.ETSGLOBAL::main"
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::test1"
//!
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 0
//! INST_COUNT    /LoadObject.*res/, 0
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::test2"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 2
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 0
//! INST_COUNT    /LoadObject.*res/, 0
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::test3"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 3
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 0
//! INST_COUNT    /LoadObject.*res/, 0
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::test4"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 6
//! INST_COUNT    /StoreObject.*res/, 6
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 0
//! INST_COUNT    /LoadObject.*res/, 0
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::test5"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 15
//! INST_COUNT    /StoreObject.*res/, 24
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 10
//! INST_COUNT    /StoreObject.*res/, 14
//! INST_COUNT    /LoadObject.*res/, 1
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::test9"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 10
//! INST_COUNT    /StoreObject.*done/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 9
//! INST_COUNT    /StoreObject.*done/, 0
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::ntest1"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::ntest2"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::ntest3"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 5
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::ntest4"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 6
//! INST_COUNT    /StoreObject.*res/, 6
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 6
//! INST_COUNT    /StoreObject.*res/, 6
//! INST_COUNT    /LoadObject.*res/, 1


//! CHECKER       PEA_propagate_load_objects_upwards AOT
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*::.?test.*'"
//! RUN           entry: "ets_pea_loads_upward_propagation.ETSGLOBAL::main"
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::test1"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject.*res/, 0
//! INST_COUNT    /LoadObject.*res/, 0
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::test2"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 2
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject.*res/, 0
//! INST_COUNT    /LoadObject.*res/, 0
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::test3"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 3
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject.*res/, 0
//! INST_COUNT    /LoadObject.*res/, 0
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::test4"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 4
//! INST_COUNT    /StoreObject.*res/, 6
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject.*res/, 0
//! INST_COUNT    /LoadObject.*res/, 0
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::test5"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 13
//! INST_COUNT    /StoreObject.*res/, 24
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 8
//! INST_COUNT    /StoreObject.*res/, 14
//! INST_COUNT    /LoadObject.*res/, 1
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::ntest1"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::ntest2"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::ntest3"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 3
//! INST_COUNT    /StoreObject.*res/, 4
//! INST_COUNT    /LoadObject.*res/, 1
//!
//! METHOD        "ets_pea_loads_upward_propagation.ETSGLOBAL::ntest4"
//! PASS_BEFORE   "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 4
//! INST_COUNT    /StoreObject.*res/, 6
//! INST_COUNT    /LoadObject.*res/, 1
//! PASS_AFTER    "EscapeAnalysis"
//! INST_COUNT    /NewObject/, 4
//! INST_COUNT    /StoreObject.*res/, 6
//! INST_COUNT    /LoadObject.*res/, 1

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
function test1(): boolean {
    let aa: Array<number> = new Array<number>
    aa.push(3);
    return foo1(aa[0]).res as int == 3;
}

/* two loads case (two phi users) */
function test2(): boolean {
    let aa: Array<number> = new Array<number>
    aa.push(3)
    let z: BBB = foo1(aa[0])
    return (z.res + z.res1) as int == 6;
}

/* more complex CFG: different number of loads in different branchs */
function test3(): boolean {
    let aa: Array<number> = new Array<number>
    aa.push(3)
    let t: boolean = (aa[0] > 0)
    let z: BBB = foo1(aa[0])
    if (t) {
        return z.res as int == 3
    } else {
        return (z.res + z.res1) as int == 6;
    }
}

/* many inputs phi */
function test4(): boolean {
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
        return true
    }
    return false;
}

/* with phi with inputs comping from and beyond
 * the loop only a partial scalarization occurs */
function test5(): boolean {
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
        return true
    }
    return false;
}

/* test the stdlib iterator */
function test9(): boolean {
    let aa: Array<number> = new Array<number>
    aa.push(3);
    aa.push(5);
    aa.push(1);
    return foo3(aa) as int == 9;
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
function ntest1(): boolean {
    let aa: Array<number> = new Array<number>
    aa.push(3)
    let z: BBB = foo1(aa[0])
    if (getRef(z)) {
        return true
    }
    return false
}

/* negative: there is a user that is not a LoadObject */
function ntest2(): boolean {
    let aa: Array<number> = new Array<number>
    aa.push(3)
    let z: BBB = foo1(aa[0])
    let res: number = z.res
    if (getRef(z)) {
        return res as int == 3;
    }
    return false
}

/* negative: the object types are different */
function ntest3(): boolean {
    let aa: Array<number> = new Array<number>
    aa.push(3);
    return foo2(aa[0]).res as int == 3;
}

/* negative: many inputs phi, different object types */
function ntest4(): boolean {
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
        return true
    }
    return false;
}

function assert(condition: boolean, message: string) {
    if (!condition) {
        throw new AssertionError('\nERROR: ' + message + '\n');
    }
}

function main(): int {
    try {
        assert(test1(), 'one load case failed');
        assert(test2(), 'two loads case failed');
        assert(test3(), 'uneven branches case failed');
        assert(test4(), 'many inputs phi case failed');
        assert(test5(), 'partial scalarization test failed');
        assert(test9(), 'stdlib iterator case failed');
        assert(ntest1(), 'not a LoadObject user case failed');
        assert(ntest2(), 'one of users is not a LoadObject case failed');
        assert(ntest3(), 'object types are different case failed');
        assert(ntest4(), 'object types are different complex case failed');
    } catch (e) {
        return 1;
    }
    return 0;
}
