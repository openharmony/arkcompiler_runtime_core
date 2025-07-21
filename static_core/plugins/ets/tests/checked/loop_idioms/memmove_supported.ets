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

function check(arr1: Object, arr2: Object): int {
    if (arr1.toString() != arr2.toString()) {
        console.print("Expected " + arr2.toString() + " but got " + arr1.toString());
        return 1;
    }
    return 0;
}

let GLOBAL_OFFS: int = 1;

class TestSupported {
    //! CHECKER       Test1
    //! RUN           force_jit: true, entry: "memmove_supported.TestSupported::noAlias_locals"
    //! METHOD        "memmove_supported.TestSupported::noAlias_locals"
    //! PASS_AFTER    "LoopIdioms"
    //! INST          /Intrinsic.*CompilerMemmoveUnchecked/
    static noAlias_locals(): int {
        let src: FixedArray<int> = [1, 2, 3, 4];
        let dst: FixedArray<int> = new int[4];
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return check(dst, [1, 2, 3, 4]);
    }

    //! CHECKER       Test2
    //! RUN           force_jit: true, entry: "memmove_supported.TestSupported::noAlias_localAndParam"
    //! METHOD        "memmove_supported.TestSupported::__noinline__noAlias_localAndParam"
    //! PASS_AFTER    "LoopIdioms"
    //! INST          /Intrinsic.*CompilerMemmoveUnchecked/
    static noAlias_localAndParam(): int {
        let src: FixedArray<int> = [1, 2, 3, 4];
        return check(TestSupported.__noinline__noAlias_localAndParam(src), src);
    }
    static __noinline__noAlias_localAndParam(src: FixedArray<int>): FixedArray<int> {
        let dst: FixedArray<int> = new int[4];
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return dst;
    }

    //! CHECKER       Test3
    //! RUN           force_jit: true, entry: "memmove_supported.TestSupported::patternMatch_offsets1"
    //! METHOD        "memmove_supported.TestSupported::patternMatch_offsets1"
    //! PASS_AFTER    "LoopIdioms"
    //! INST          /Intrinsic.*CompilerMemmoveUnchecked/
    static patternMatch_offsets1(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[i + offs] = src[i];
        }
        return check(dst, [1, 2, 6, 7, 8]);
    }

    //! CHECKER       Test4
    //! RUN           force_jit: true, entry: "memmove_supported.TestSupported::patternMatch_offsets2"
    //! METHOD        "memmove_supported.TestSupported::patternMatch_offsets2"
    //! PASS_AFTER    "LoopIdioms"
    //! INST          /Intrinsic.*CompilerMemmoveUnchecked/
    static patternMatch_offsets2(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = offs; i < dst.length; i++) {
            dst[i] = src[i - offs];
        }
        return check(dst, [1, 2, 6, 7, 8]);
    }

    //! CHECKER       Test5
    //! RUN           force_jit: true, entry: "memmove_supported.TestSupported::patternMatch_offsets3"
    //! METHOD        "memmove_supported.TestSupported::patternMatch_offsets3"
    //! PASS_AFTER    "LoopIdioms"
    //! INST          /Intrinsic.*CompilerMemmoveUnchecked/
    static patternMatch_offsets3(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[i + offs] = src[i];
        }
        return check(dst, [1, 2, 6, 7, 8]);
    }

    //! CHECKER       Test6
    //! RUN           force_jit: true, entry: "memmove_supported.TestSupported::patternMatch_offsets4"
    //! METHOD        "memmove_supported.TestSupported::patternMatch_offsets4"
    //! PASS_AFTER    "LoopIdioms"
    //! INST          /Intrinsic.*CompilerMemmoveUnchecked/
    static patternMatch_offsets4(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = offs; i < dst.length; i++) {
            dst[i] = src[i - offs];
        }
        return check(dst, [1, 2, 6, 7, 8]);
    }

    //! CHECKER       Test7
    //! RUN           force_jit: true, entry: "memmove_supported.TestSupported::patternMatch_offsets_sameNegOffset"
    //! METHOD        "memmove_supported.TestSupported::patternMatch_offsets_sameNegOffset"
    //! PASS_AFTER    "LoopIdioms"
    //! INST          /Intrinsic.*CompilerMemmoveUnchecked/
    static patternMatch_offsets_sameNegOffset(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        for (let i: int = 1; i < src.length; i++) {
            dst[i - 1] = src[i - 1];
        }
        return check(dst, [6, 7, 3, 4, 5]);
    }

    //! CHECKER       Test8
    //! RUN           force_jit: true, entry: "memmove_supported.TestSupported::patternMatch_offsets_negOffsets"
    //! METHOD        "memmove_supported.TestSupported::patternMatch_offsets_negOffsets"
    //! PASS_AFTER    "LoopIdioms"
    //! INST          /Intrinsic.*CompilerMemmoveUnchecked/
    static patternMatch_offsets_negOffsets(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        for (let i: int = 2; i < src.length + 1; i++) {
            dst[i - 1] = src[i - 2];
        }
        return check(dst, [1, 6, 7, 4, 5]);
    }

    //! CHECKER       Test9
    //! RUN           force_jit: true, entry: "memmove_supported.TestSupported::backwardsClosed"
    //! METHOD        "memmove_supported.TestSupported::backwardsClosed"
    //! PASS_AFTER    "LoopIdioms"
    //! INST          /Intrinsic.*CompilerMemmoveUnchecked/
    static backwardsClosed(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        let x = GLOBAL_OFFS;
        for (let i: int = src.length - 1; i >= 0; i--) {
            dst[i + 1] = src[i];
        }
        return check(dst, [1, 6, 7, 8, 5]);
    }

    //! CHECKER       Test10
    //! RUN           force_jit: true, entry: "memmove_supported.TestSupported::backwardsOpen"
    //! METHOD        "memmove_supported.TestSupported::backwardsOpen"
    //! PASS_AFTER    "LoopIdioms"
    //! INST          /Intrinsic.*CompilerMemmoveUnchecked/
    static backwardsOpen(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let x = GLOBAL_OFFS;
        for (let i: int = src.length - 1; i > 0; i--) {
            dst[i - 1] = src[i];
        }
        return check(dst, [7, 8, 3, 4, 5]);
    }

    //! CHECKER       Test11
    //! RUN           force_jit: true, entry: "memmove_supported.TestSupported::forwardClosed"
    //! METHOD        "memmove_supported.TestSupported::forwardClosed"
    //! PASS_AFTER    "LoopIdioms"
    //! INST          /Intrinsic.*CompilerMemmoveUnchecked/
    static forwardClosed(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        let x = GLOBAL_OFFS;
        for (let i: int = 0; i <= src.length - 1; i++) {
            dst[i + 1] = src[i];
        }
        return check(dst, [1, 6, 7, 8, 5]);
    }
};
