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

class A {
    public valA_: int;

    constructor() {
        this.valA_ = 0
    }

    constructor(valA: int) {
        this.valA_ = valA
    }
}

class B extends A {
    public valB_: int;

    constructor(valA: int, valB: int) {
        super(valA);
        this.valB_ = valB;
    }
}

function __noinline__modify(obj: A) {
    obj.valA_ = 100500
}

function __noinline__modify(arr: int[]) {
    arr[0] = 100500
}

function __noinline__getLength(flag: boolean): int {
    return 3 + (flag ? 1 : 0);
}

//! CHECKER       AOT Escape eliminate phi with objects
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-inline-external-methods-aot=true --compiler-regex='.*test.*'"
//! METHOD        "ETSGLOBAL::testSimplePhi1"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! PASS_AFTER    "Escape"
//! INST_NOT      /NewObject/
//! INST_NOT      /StoreObject/
//! INST_NOT      /LoadObject/
//!
//! METHOD        "ETSGLOBAL::testSimplePhi2"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject/, 1
//! INST_COUNT    /LoadObject/, 1
//!
//! METHOD        "ETSGLOBAL::testWithSlowpath"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 2
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject/, 1
//! INST_COUNT    /LoadObject/, 1
//!
//! METHOD        "ETSGLOBAL::testDifferentClasses"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 3
//! INST_COUNT    /LoadObject/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 3
//! INST_COUNT    /LoadObject/, 1
//!
//! METHOD        "ETSGLOBAL::testLoop"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 7
//! INST_COUNT    /StoreObject/, 7
//! INST_COUNT    /LoadObject/, 0
//! INST_COUNT    /ref.*Phi/, 0
//! INST_COUNT    /i32.*Phi/, 3
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject/, 1
//! INST_COUNT    /LoadObject/, 0
//! INST_COUNT    /ref.*Phi/, 0
//! INST_COUNT    /i32.*Phi/, 3
//!
//! METHOD        "ETSGLOBAL::testDominateObjects"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::testDominateObjectsMaterialize"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::testDominateObjectsMaterializeInSlowpath"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 2
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 2
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::testDominateObjectsSeparateObjUsage"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::testTempArrays"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 9
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 9
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::testTempArraysDiffLength1"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 14
//! INST_COUNT    /LoadArray/, 11
//! INST_COUNT    /ref.*Phi/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 14
//! INST_COUNT    /LoadArray/, 11
//! INST_COUNT    /ref.*Phi/, 1
//!
//! METHOD        "ETSGLOBAL::testTempArraysDiffLength2"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 2
//! INST_COUNT    /StoreArray/, 20
//! INST_COUNT    /LoadArray/, 4
//! INST_COUNT    /ref.*Phi/, 5
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 2
//! INST_COUNT    /StoreArray/, 20
//! INST_COUNT    /LoadArray/, 4
//! INST_COUNT    /ref.*Phi/, 5
//!
//! METHOD        "ETSGLOBAL::testArraysMaterialize"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 9
//! INST_COUNT    /ref.*Phi/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 9
//! INST_COUNT    /ref.*Phi/, 1
//!
//! METHOD        "ETSGLOBAL::testArraysMaterializeInSlowpath"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 12
//! INST_COUNT    /ref.*Phi/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 12
//! INST_COUNT    /ref.*Phi/, 1

//! CHECKER       JIT Escape eliminate phi with objects
//! RUN           force_jit: true, options: "--compiler-regex='.*test.*'", entry: "escape_phi.ETSGLOBAL::main"
//! METHOD        "ETSGLOBAL::testSimplePhi1"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! PASS_AFTER    "Escape"
//! INST_NOT      /NewObject/
//! INST_NOT      /StoreObject/
//! INST_NOT      /LoadObject/
//!
//! METHOD        "ETSGLOBAL::testSimplePhi2"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject/, 1
//! INST_COUNT    /LoadObject/, 1
//!
//! METHOD        "ETSGLOBAL::testWithSlowpath"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 2
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject/, 1
//! INST_COUNT    /LoadObject/, 1
//!
//! METHOD        "ETSGLOBAL::testDifferentClasses"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 3
//! INST_COUNT    /LoadObject/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 3
//! INST_COUNT    /LoadObject/, 1
//!
//! METHOD        "ETSGLOBAL::testLoop"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 7
//! INST_COUNT    /StoreObject/, 7
//! INST_COUNT    /LoadObject/, 0
//! INST_COUNT    /ref.*Phi/, 0
//! INST_COUNT    /i32.*Phi/, 3
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject/, 1
//! INST_COUNT    /LoadObject/, 0
//! INST_COUNT    /ref.*Phi/, 0
//! INST_COUNT    /i32.*Phi/, 3
//!
//! METHOD        "ETSGLOBAL::testDominateObjects"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::testDominateObjectsMaterialize"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::testDominateObjectsMaterializeInSlowpath"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 2
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 2
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::testDominateObjectsSeparateObjUsage"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::testTempArrays"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 9
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 9
//! INST_COUNT    /ref.*Phi/, 1
//! INST_COUNT    /i32.*Phi/, 0
//!
//! METHOD        "ETSGLOBAL::testTempArraysDiffLength1"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 14
//! INST_COUNT    /LoadArray/, 11
//! INST_COUNT    /ref.*Phi/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 14
//! INST_COUNT    /LoadArray/, 11
//! INST_COUNT    /ref.*Phi/, 1
//!
//! METHOD        "ETSGLOBAL::testTempArraysDiffLength2"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 2
//! INST_COUNT    /StoreArray/, 24
//! INST_COUNT    /LoadArray/, 4
//! INST_COUNT    /ref.*Phi/, 5
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 2
//! INST_COUNT    /StoreArray/, 24
//! INST_COUNT    /LoadArray/, 4
//! INST_COUNT    /ref.*Phi/, 5
//!
//! METHOD        "ETSGLOBAL::testArraysMaterialize"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 9
//! INST_COUNT    /ref.*Phi/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 9
//! INST_COUNT    /ref.*Phi/, 1
//!
//! METHOD        "ETSGLOBAL::testArraysMaterializeInSlowpath"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 12
//! INST_COUNT    /ref.*Phi/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 12
//! INST_COUNT    /ref.*Phi/, 1


// apply, objects are temporary
function testSimplePhi1(a: int, b: int): int {
    let obj: A;
    if (a > b) {
        obj = new A(a);
    } else {
        obj = new A(b);
    }
    return obj.valA_;
}

// no apply, object escapes
function testSimplePhi2(a: int, b: int): int {
    let obj: A;
    if (a > b) {
        obj = new A(a);
    } else {
        obj = new A(b);
    }
    __noinline__modify(obj);
    return obj.valA_;
}

// apply, materialize into slowpath
function testWithSlowpath(a: int, b: int, flag: boolean): int {
    let obj: A;
    if (a > b) {
        obj = new A(a);
    } else {
        obj = new A(b);
    }
    if (flag) {
        __noinline__modify(obj);
        return obj.valA_;
    }
    return obj.valA_;
}

// no apply, different classes
function testDifferentClasses(a: int, b: int): int {
    let obj: A;
    if (a > b) {
        obj = new A(a);
    } else {
        obj = new B(b, a);
    }
    return obj.valA_;
}

// apply, objects are temporary
function testLoop(n: int) : int {
    let a = new A(0);
    while (a.valA_ != n) {
        a = new A(a.valA_ + 1);
    }
    return a.valA_;
}

// no apply
function testDominateObjects(flag: boolean): int {
    let a1 = new A(0);
    let a2 = new A(1);
    let a: A;
    if (flag) {
        a = a1;
    } else {
        a = a2;
    }
    return a.valA_;
}

// no apply, objects are temporary, 
function testDominateObjectsSeparateObjUsage(flag: boolean): int {
    let a1 = new A(0);
    let a2 = new A(1);
    let a: A;
    if (flag) {
        a = a1;
    } else {
        a = a2;
    }
    a2.valA_ = 3;
    return a.valA_;
}

// no apply
function testDominateObjectsMaterialize(flag: boolean): int {
    let a1 = new A(0);
    let a2 = new A(1);
    let a: A;
    if (flag) {
        a = a1;
    } else {
        a = a2;
    }
    __noinline__modify(a);
    return a.valA_;
}

// no apply
function testDominateObjectsMaterializeInSlowpath(flag: boolean, slowpath: boolean): int {
    let a1 = new A(0);
    let a2 = new A(1);
    let a: A;
    if (flag) {
        a = a1;
    } else {
        a = a2;
    }
    if (slowpath) {
        __noinline__modify(a);
        return a.valA_;
    }
    return a.valA_;
}

// apply
function testTempArrays(flag: boolean): int {
    let arr: int[];
    if (flag) {
        arr = [0, 1, 2];
    } else {
        arr = [3, 4, 5];
    }
    return arr[0] + arr[1] + arr[2];
}

// no apply, different length
function testTempArraysDiffLength1(flag: boolean): int {
    let arr: int[];
    if (flag) {
        arr = [0, 1, 2, 4];
    } else {
        arr = [3, 4, 5];
    }
    return arr[0] + arr[1] + arr[2] + arr[3];
}

// no apply, different length
function testTempArraysDiffLength2(flag: boolean): int {
    let length = __noinline__getLength(flag);
    let arr: int[];
    if (flag) {
        arr = new int[length];
        for (let i: int = 0; i < length; ++i) {
            arr[i] = i +(flag ? 1 : 0);
        }
    } else {
        arr = new int[length];
        for (let i: int = 0; i < length; ++i) {
            arr[i] = i;
        }
    }
    return arr[0] + arr[1] + arr[2] + arr[3];
}

// no apply
function testArraysMaterialize(flag: boolean): int {
    let arr: int[];
    if (flag) {
        arr = [0, 1, 2];
    } else {
        arr = [3, 4, 5];
    }
    __noinline__modify(arr);
    return arr[0] + arr[1] + arr[2];
}

// apply
function testArraysMaterializeInSlowpath(flag1: boolean, flag2: boolean): int {
    let arr: int[];
    if (flag1) {
        arr = [0, 1, 2];
    } else {
        arr = [3, 4, 5];
    }
    if (flag2) {
        __noinline__modify(arr);
        return arr[0] + arr[1] + arr[2];
    }
    return arr[0] + arr[1] + arr[2];
}

function main(): int {
    if (testSimplePhi1(1, 2) != 2) {
        return 1;
    }
    if (testSimplePhi1(2, 1) != 2) {
        return 2;
    }
    if (testSimplePhi2(1, 2) != 100500) {
        return 3;
    }
    if (testSimplePhi2(2, 1) != 100500) {
        return 4;
    }
    if (testWithSlowpath(1, 2, false) != 2) {
        return 5;
    }
    if (testWithSlowpath(2, 1, false) != 2) {
        return 6;
    }
    if (testWithSlowpath(2, 1, true) != 100500) {
        return 7;
    }
    if (testDifferentClasses(1, 2) != 2) {
        return 8;
    }
    if (testDifferentClasses(2, 1) != 2) {
        return 9;
    }
    if (testLoop(10) != 10) {
        return 10;
    }
    if (testDominateObjects(true) != 0) {
        return 11;
    }
    if (testDominateObjects(false) != 1) {
        return 12;
    }
    if (testDominateObjectsMaterialize(true) != 100500) {
        return 13;
    }
    if (testDominateObjectsMaterialize(false) != 100500) {
        return 14;
    }
    if (testDominateObjectsMaterializeInSlowpath(false, false) != 1) {
        return 15;
    }
    if (testDominateObjectsMaterializeInSlowpath(true, false) != 0) {
        return 16;
    }
    if (testDominateObjectsMaterializeInSlowpath(false, true) != 100500) {
        return 17;
    }
    if (testDominateObjectsMaterializeInSlowpath(true, true) != 100500) {
        return 18;
    }
    if (testTempArrays(true) != 3) {
        return 19;
    }
    if (testTempArrays(false) != 12) {
        return 20;
    }
    if (testTempArraysDiffLength1(true) != 7) {
        return 21;
    }
    try {
        testTempArraysDiffLength1(false);
        return 22;
    } catch (e) {}
    if (testTempArraysDiffLength2(true) != 10) {
        return 23;
    }
    try {
        testTempArraysDiffLength2(false);
        return 24;
    } catch (e) {}
    if (testArraysMaterialize(true) != 100503) {
        return 25;
    }
    if (testArraysMaterialize(false) != 100509) {
        return 26;
    }
    if (testArraysMaterializeInSlowpath(false, false) != 12) {
        return 27;
    }
    if (testArraysMaterializeInSlowpath(true, false) != 3) {
        return 28;
    }
    if (testArraysMaterializeInSlowpath(false, true) != 100509) {
        return 29;
    }
    if (testArraysMaterializeInSlowpath(true, true) != 100503) {
        return 30;
    }
    if (testDominateObjectsSeparateObjUsage(true) != 0) {
        return 31;
    }
    if (testDominateObjectsSeparateObjUsage(false) != 3) {
        return 32;
    }
    return 0;
}
