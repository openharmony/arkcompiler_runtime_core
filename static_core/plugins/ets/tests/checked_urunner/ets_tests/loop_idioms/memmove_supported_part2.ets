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
//! RUN           force_jit: true, options: "--compiler-regex='.*memmove_supported_part2.TestSupported::.*test.*'",
//!               entry: "memmove_supported_part2.ETSGLOBAL::main"
//!
//! METHOD        "memmove_supported_part2.TestSupported::testNoAliasLocals"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported_part2.TestSupported::__noinline__testNoAliasLocalAndParam"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported_part2.TestSupported::testBackwardsClosed"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported_part2.TestSupported::testBackwardsOpen"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported_part2.TestSupported::testForwardClosed"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported_part2.TestSupported::testForwardClosedRef"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported_part2.TestSupported::testBackwardsOpenRef"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported_part2.TestSupported::testBackwardsClosedRef"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported_part2.TestSupported::testNoAloasLocalsRef"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported_part2.TestSupported::testNoAliasUpdateFirstOnlyRef"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/

//! CHECKER       AOT Test memmove for references
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*memmove_supported_part2.TestSupported::.*test.*'"
//!
//! METHOD        "memmove_supported_part2.TestSupported::testForwardClosedRef"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported_part2.TestSupported::testBackwardsOpenRef"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported_part2.TestSupported::testBackwardsClosedRef"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported_part2.TestSupported::testNoAloasLocalsRef"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported_part2.TestSupported::testNoAliasUpdateFirstOnlyRef"
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
    static testNoAliasLocals(): int {
        let src: ValueArray<int> = [1, 2, 3, 4];
        let dst: ValueArray<int> = new ValueArray<int>(4);
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return check(dst, [1, 2, 3, 4]);
    }

    static testNoAloasLocalsRef(): int {
        let src: FixedArray<string> = ['1', '2', '3', '4'];
        let dst: FixedArray<string> = new FixedArray<string>(4);
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return check(dst, ['1', '2', '3', '4']);
    }

    static testNoAliasUpdateFirstOnlyRef(): int {
        let src: FixedArray<string> = ['1', '2', '3', '4'];
        let dst: FixedArray<string> = new FixedArray<string>(4);
        TestSupported.__noinline__updateFirst(dst);
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return check(dst, ['1', '2', '3', '4']);
    }

    static __noinline__updateFirst(a: FixedArray<string>) {
        a[0] = '';
    }

    static testNoAliasLocalAndParam(): int {
        let src: ValueArray<int> = [1, 2, 3, 4];
        return check(TestSupported.__noinline__testNoAliasLocalAndParam(src), src);
    }

    static __noinline__testNoAliasLocalAndParam(src: ValueArray<int>): ValueArray<int> {
        let dst: ValueArray<int> = new ValueArray<int>(4);
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return dst;
    }

    static testBackwardsClosed(): int {
        let src: ValueArray<int> = [6, 7, 8];
        let dst: ValueArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        let x = GLOBAL_OFFS;
        for (let i: int = src.length - 1; i >= 0; i--) {
            dst[i + 1] = src[i];
        }
        return check(dst, [1, 6, 7, 8, 5]);
    }

    static testBackwardsClosedRef(): int {
        let src: FixedArray<string> = ['6', '7', '8'];
        let dst: FixedArray<string> = ['1', '2', '3', '4', '5'];
        for (let i: int = src.length - 1; i >= 0; i--) {
            dst[i + 1] = src[i];
        }
        return check(dst, ['1', '6', '7', '8', '5']);
    }

    static testBackwardsOpen(): int {
        let src: ValueArray<int> = [6, 7, 8];
        let dst: ValueArray<int> = [1, 2, 3, 4, 5];
        let x = GLOBAL_OFFS;
        for (let i: int = src.length - 1; i > 0; i--) {
            dst[i - 1] = src[i];
        }
        return check(dst, [7, 8, 3, 4, 5]);
    }

    static testBackwardsOpenRef(): int {
        let src: FixedArray<string> = ['6', '7', '8'];
        let dst: FixedArray<string> = ['1', '2', '3', '4', '5'];
        for (let i: int = src.length - 1; i > 0; i--) {
            dst[i - 1] = src[i];
        }
        return check(dst, ['7', '8', '3', '4', '5']);
    }

    static testForwardClosed(): int {
        let src: ValueArray<int> = [6, 7, 8];
        let dst: ValueArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        let x = GLOBAL_OFFS;
        for (let i: int = 0; i <= src.length - 1; i++) {
            dst[i + 1] = src[i];
        }
        return check(dst, [1, 6, 7, 8, 5]);
    }

    static testForwardClosedRef(): int {
        let src: FixedArray<string> = ['6', '7', '8'];
        let dst: FixedArray<string> = ['1', '2', '3', '4', '5'];
        for (let i: int = 0; i <= src.length - 1; i++) {
            dst[i + 1] = src[i];
        }
        return check(dst, ['1', '6', '7', '8', '5']);
    }
};

function main(): int {
    let res: int = 0;
    res += TestSupported.testNoAliasLocals();
    res += TestSupported.testNoAloasLocalsRef();
    res += TestSupported.testNoAliasLocalAndParam();
    res += TestSupported.testNoAliasUpdateFirstOnlyRef();
    res += TestSupported.testBackwardsClosed();
    res += TestSupported.testBackwardsClosedRef();
    res += TestSupported.testBackwardsOpen();
    res += TestSupported.testBackwardsOpenRef();
    res += TestSupported.testForwardClosed();
    res += TestSupported.testForwardClosedRef();
    arktest.assertEQ(res, 0);
    return res;
}
