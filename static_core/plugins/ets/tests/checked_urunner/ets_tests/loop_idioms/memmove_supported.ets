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

//! CHECKER       Test memmove
//! RUN           force_jit: true,
//!               options: "--compiler-regex='.*memmove_supported.TestSupported::testPatternMatchOffsets.*'",
//!               entry: "memmove_supported.ETSGLOBAL::main"
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsets1"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsets2"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsets3"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsets4"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsetsSameNegOffset"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsetsNegOffsets"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsetsNegOffsetsRef"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsetsSameNegOffsetRef"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsets4Ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsets3Ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsets2Ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsets1Ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/

//! CHECKER       AOT Test memmove for references
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*memmove_supported.TestSupported::testPatternMatchOffsets.*'"
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsetsNegOffsetsRef"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsetsSameNegOffsetRef"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsets4Ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsets3Ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsets2Ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::testPatternMatchOffsets1Ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/

function check(arr1: Object, arr2: Object): int {
    if (arr1.toString() != arr2.toString()) {
        console.print('Expected ' + arr2.toString() + ' but got ' + arr1.toString());
        return 1;
    }
    return 0;
}

let GLOBAL_OFFS: int = 1;

class TestSupported {
    static testPatternMatchOffsets1(): int {
        let src: ValueArray<int> = [6, 7, 8];
        let dst: ValueArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[i + offs] = src[i];
        }
        return check(dst, [1, 2, 6, 7, 8]);
    }

    static testPatternMatchOffsets1Ref(): int {
        let src: FixedArray<string> = ['6', '7', '8'];
        let dst: FixedArray<string> = ['1', '2', '3', '4', '5'];
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[i + offs] = src[i];
        }
        return check(dst, ['1', '2', '6', '7', '8']);
    }

    static testPatternMatchOffsets2(): int {
        let src: ValueArray<int> = [6, 7, 8];
        let dst: ValueArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = offs; i < dst.length; i++) {
            dst[i] = src[i - offs];
        }
        return check(dst, [1, 2, 6, 7, 8]);
    }

    static testPatternMatchOffsets2Ref(): int {
        let src: FixedArray<string> = ['6', '7', '8'];
        let dst: FixedArray<string> = ['1', '2', '3', '4', '5'];
        let offs = dst.length - src.length;
        for (let i: int = offs; i < dst.length; i++) {
            dst[i] = src[i - offs];
        }
        return check(dst, ['1', '2', '6', '7', '8']);
    }

    static testPatternMatchOffsets3(): int {
        let src: ValueArray<int> = [6, 7, 8];
        let dst: ValueArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[i + offs] = src[i];
        }
        return check(dst, [1, 2, 6, 7, 8]);
    }

    static testPatternMatchOffsets3Ref(): int {
        let src: FixedArray<string> = ['6', '7', '8'];
        let dst: FixedArray<string> = ['1', '2', '3', '4', '5'];
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[i + offs] = src[i];
        }
        return check(dst, ['1', '2', '6', '7', '8']);
    }

    static testPatternMatchOffsets4(): int {
        let src: ValueArray<int> = [6, 7, 8];
        let dst: ValueArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = offs; i < dst.length; i++) {
            dst[i] = src[i - offs];
        }
        return check(dst, [1, 2, 6, 7, 8]);
    }

    static testPatternMatchOffsets4Ref(): int {
        let src: FixedArray<string> = ['6', '7', '8'];
        let dst: FixedArray<string> = ['1', '2', '3', '4', '5'];
        let offs = dst.length - src.length;
        for (let i: int = offs; i < dst.length; i++) {
            dst[i] = src[i - offs];
        }
        return check(dst, ['1', '2', '6', '7', '8']);
    }

    static testPatternMatchOffsetsSameNegOffset(): int {
        let src: ValueArray<int> = [6, 7, 8];
        let dst: ValueArray<int> = [1, 2, 3, 4, 5];
        for (let i: int = 1; i < src.length; i++) {
            dst[i - 1] = src[i - 1];
        }
        return check(dst, [6, 7, 3, 4, 5]);
    }

    static testPatternMatchOffsetsSameNegOffsetRef(): int {
        let src: FixedArray<string> = ['6', '7', '8'];
        let dst: FixedArray<string> = ['1', '2', '3', '4', '5'];
        for (let i: int = 1; i < src.length; i++) {
            dst[i - 1] = src[i - 1];
        }
        return check(dst, ['6', '7', '3', '4', '5']);
    }

    static testPatternMatchOffsetsNegOffsets(): int {
        let src: ValueArray<int> = [6, 7, 8];
        let dst: ValueArray<int> = [1, 2, 3, 4, 5];
        for (let i: int = 2; i < src.length + 1; i++) {
            dst[i - 1] = src[i - 2];
        }
        return check(dst, [1, 6, 7, 4, 5]);
    }

    static testPatternMatchOffsetsNegOffsetsRef(): int {
        let src: FixedArray<string> = ['6', '7', '8'];
        let dst: FixedArray<string> = ['1', '2', '3', '4', '5'];
        for (let i: int = 2; i < src.length + 1; i++) {
            dst[i - 1] = src[i - 2];
        }
        return check(dst, ['1', '6', '7', '4', '5']);
    }
};

function main(): int {
    let res: int = 0;
    res += TestSupported.testPatternMatchOffsets1();
    res += TestSupported.testPatternMatchOffsets1Ref();
    res += TestSupported.testPatternMatchOffsets2();
    res += TestSupported.testPatternMatchOffsets2Ref();
    res += TestSupported.testPatternMatchOffsets3();
    res += TestSupported.testPatternMatchOffsets3Ref();
    res += TestSupported.testPatternMatchOffsets4();
    res += TestSupported.testPatternMatchOffsets4Ref();
    res += TestSupported.testPatternMatchOffsetsSameNegOffset();
    res += TestSupported.testPatternMatchOffsetsSameNegOffsetRef();
    res += TestSupported.testPatternMatchOffsetsNegOffsets();
    res += TestSupported.testPatternMatchOffsetsNegOffsetsRef();
    arktest.assertEQ(res, 0);
    return res;
}
