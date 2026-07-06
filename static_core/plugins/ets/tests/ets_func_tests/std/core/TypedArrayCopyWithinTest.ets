/**
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

/*
 * Tests for the fixed copyWithin(target) 1-arg overload behavior.
 *
 * Background: the 1-arg overload previously called native copyWithinImpl(target, 0, length)
 * directly, bypassing the normalizeIndex + clamp the 3-arg overload applies. A negative or
 * oversized target reached native code out-of-contract and caused a SIGSEGV in memmove_s. The
 * fix makes the 1-arg overload delegate to the validated 3-arg overload
 * (return this.copyWithin(target, 0, this.lengthInt)), so target goes through full ES-spec
 * index normalization. These tests verify the fix holds across all typed-array classes and the
 * full set of index cases.
 */

// ----------------------------------------------------------------------------
// Primary: copyWithin(-1) no longer crashes; from-end semantics correct
// ----------------------------------------------------------------------------

function testCopyWithinNegativeTargetFromEnd(): void {
    // copyWithin(-1) on [0..9]: target normalizes to index 9 (from-end). Copies [0..10)
    // (start=0, end=length=10) onto [9..10) -> only index 9 is writable, gets source[0]=0.
    let a = new Int8Array(10);
    for (let i = 0; i < 10; i++) { a[i] = i; }
    a.copyWithin(-1);
    arktest.assertEQ(a[9], 0, 'copyWithin(-1) writes source[0] to index 9 (from-end)');
    // Indices 0..8 are untouched.
    arktest.assertEQ(a[0], 0, 'index 0 untouched');
    arktest.assertEQ(a[8], 8, 'index 8 untouched');

    // Equivalence: copyWithin(-1) must equal copyWithin(length-1, 0, length).
    let b = new Int8Array(10);
    for (let i = 0; i < 10; i++) { b[i] = i; }
    b.copyWithin(9, 0, 10);
    arktest.assertEQ(a[9], b[9], 'copyWithin(-1) == copyWithin(length-1,0,length)');
}

// ----------------------------------------------------------------------------
// Full normalizeIndex + clamp semantics + coverage across all typed-array classes
// ----------------------------------------------------------------------------

function testCopyWithinNormalizationSemantics(): void {
    // ---- ES-spec index normalization on a single class (Int8Array, length 6) ----
    // copyWithin(t) must equal copyWithin(t, 0, length) for every `t` category.

    // (a) negative target with magnitude < length -> from-end index.
    //     copyWithin(-3) on [10,20,30,40,50,60] == copyWithin(3,0,6): copies [0..6) onto [3..6).
    let aNeg = new Int8Array(6);
    for (let i = 0; i < 6; i++) { 
        aNeg[i] = (i + 1) * 10; // result will be [10,20,30,40,50,60]
    }
    let aNegRef = new Int8Array(6);
    for (let i = 0; i < 6; i++) { aNegRef[i] = (i + 1) * 10; }
    aNeg.copyWithin(-3);
    aNegRef.copyWithin(3, 0, 6);
    for (let i = 0; i < 6; i++) {
        arktest.assertEQ(aNeg[i], aNegRef[i], 'copyWithin(-3) == copyWithin(3,0,6) at ' + i);
    }
    // After copyWithin(-3): [10,20,30,10,20,30]
    arktest.assertEQ(aNeg[3], 10, 'copyWithin(-3) puts source[0] at index 3');
    arktest.assertEQ(aNeg[5], 30, 'copyWithin(-3) puts source[2] at index 5');

    // (b) negative target with magnitude >= length -> clamps to 0 (copy onto start).
    //     copyWithin(-100) on [1,2,3] == copyWithin(0,0,3): no-op-ish whole-array self-copy.
    let aBig = new Int8Array(3);
    aBig[0] = 1; aBig[1] = 2; aBig[2] = 3;
    aBig.copyWithin(-100);
    arktest.assertEQ(aBig[0], 1, 'copyWithin(-100) clamps target to 0, [0] unchanged');
    arktest.assertEQ(aBig[2], 3, 'copyWithin(-100) clamps target to 0, [2] unchanged');

    // (c) target >= length -> no-op (nothing to copy onto).
    let aOob = new Int8Array(4);
    for (let i = 0; i < 4; i++) { 
        // CC-OFFNXT(G.OTH.03) test logic
        aOob[i] = (i + 1) * 5; // [5,10,15,20]
    }
    aOob.copyWithin(4); // == length -> no-op
    aOob.copyWithin(999); // > length -> no-op
    arktest.assertEQ(aOob[0], 5, 'copyWithin(>=length) is a no-op, [0] unchanged');
    arktest.assertEQ(aOob[3], 20, 'copyWithin(>=length) is a no-op, [3] unchanged');

    // (d) zero target -> copies whole array onto itself from index 0.
    let aZero = new Int8Array(3);
    aZero[0] = 7; aZero[1] = 8; aZero[2] = 9;
    aZero.copyWithin(0);
    arktest.assertEQ(aZero[0], 7, 'copyWithin(0) self-copy leaves [0] intact');
    arktest.assertEQ(aZero[2], 9, 'copyWithin(0) self-copy leaves [2] intact');
}

function testCopyWithinNegativeTargetAllClasses(): void {
    // Regression guard across every typed-array class. Each previously reached native
    // out-of-contract with a negative target; verify from-end semantics now hold everywhere.
    let u8 = new Uint8Array(8);
    for (let i = 0; i < 8; i++) { u8[i] = i; }
    u8.copyWithin(-1);
    arktest.assertEQ(u8[7], 0, 'Uint8Array copyWithin(-1)');

    let i16 = new Int16Array(5);
    for (let i = 0; i < 5; i++) { i16[i] = i; }
    i16.copyWithin(-1);
    arktest.assertEQ(i16[4], 0, 'Int16Array copyWithin(-1)');

    let u8c = new Uint8ClampedArray(5);
    for (let i = 0; i < 5; i++) { u8c[i] = i; }
    u8c.copyWithin(-1);
    arktest.assertEQ(u8c[4], 0, 'Uint8ClampedArray copyWithin(-1)');

    let u16 = new Uint16Array(5); for (let i = 0; i < 5; i++) { u16[i] = i; }
    u16.copyWithin(-1);
    arktest.assertEQ(u16[4], 0, 'Uint16Array copyWithin(-1)');

    let i32 = new Int32Array(4); for (let i = 0; i < 4; i++) { i32[i] = i; }
    i32.copyWithin(-1);
    arktest.assertEQ(i32[3], 0, 'Int32Array copyWithin(-1)');

    let u32 = new Uint32Array(5); for (let i = 0; i < 5; i++) { u32[i] = i; }
    u32.copyWithin(-1);
    arktest.assertEQ(u32[4], 0, 'Uint32Array copyWithin(-1)');

    let b64 = new BigInt64Array(5); for (let i = 0; i < 5; i++) { b64[i] = new BigInt(i); }
    b64.copyWithin(-1);
    arktest.assertTrue(b64[4].equals(new BigInt(0)), 'BigInt64Array copyWithin(-1)');

    let bu64 = new BigUint64Array(5); for (let i = 0; i < 5; i++) { bu64[i] = new BigInt(i); }
    bu64.copyWithin(-1);
    arktest.assertTrue(bu64[4].equals(new BigInt(0)), 'BigUint64Array copyWithin(-1)');

    let f32 = new Float32Array(5); for (let i = 0; i < 5; i++) { f32[i] = i; }
    f32.copyWithin(-1);
    arktest.assertEQ(f32[4], 0, 'Float32Array copyWithin(-1)');

    let f64 = new Float64Array(4);
    for (let i = 0; i < 4; i++) { f64[i] = i; }
    f64.copyWithin(-1);
    arktest.assertEQ(f64[3], 0, 'Float64Array copyWithin(-1)');
}

// ----------------------------------------------------------------------------

function main(): int {
    const suite = new arktest.ArkTestsuite('TypedArray copyWithin 1-arg normalization tests');
    suite.addTest('copyWithin(-1) from-end semantics', testCopyWithinNegativeTargetFromEnd);
    suite.addTest('copyWithin normalizeIndex semantics', testCopyWithinNormalizationSemantics);
    suite.addTest('copyWithin(-1) across all typed-array classes', testCopyWithinNegativeTargetAllClasses);
    return suite.run();
}
