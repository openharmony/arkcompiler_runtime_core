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

function check(arr1: Object, arr2: Object): boolean {
    if (arr1.toString() != arr2.toString()) {
        console.print("Expected " + arr2.toString() + " but got " + arr1.toString());
        return true;
    }
    return false;
}

class TestBailout {
    //! CHECKER       TestBailout1
    //! RUN           force_jit: true, entry: "memmove_bailout.TestBailout::mayAlias"
    //! METHOD        "memmove_bailout.TestBailout::mayAlias"
    //! PASS_AFTER    "LoopIdioms"
    //! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/
    static mayAlias(): boolean {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        return check(TestBailout.__noinline__mayAlias(src, dst), [6, 7, 8, 4, 5]);
    }

    static __noinline__mayAlias(src: FixedArray<int>, dst: FixedArray<int>): FixedArray<int> {
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return dst;
    }

    //! CHECKER       TestBailout2
    //! RUN           force_jit: true, entry: "memmove_bailout.TestBailout::patternMissmatch_offsets1"
    //! METHOD        "memmove_bailout.TestBailout::patternMissmatch_offsets1"
    //! PASS_AFTER    "LoopIdioms"
    //! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/
    static patternMissmatch_offsets1(): boolean {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[src.length - 1 - i];
        }
        return check(dst, [8, 7, 6, 4, 5]);
    }

    //! CHECKER       TestBailout3
    //! RUN           force_jit: true, entry: "memmove_bailout.TestBailout::patternMissmatch_offsets2"
    //! METHOD        "memmove_bailout.TestBailout::patternMissmatch_offsets2"
    //! PASS_AFTER    "LoopIdioms"
    //! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/
    static patternMissmatch_offsets2(): boolean {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[dst.length - 1 - i] = src[i];
        }
        return check(dst, [1, 2, 8, 7, 6]);
    }

    //! CHECKER       TestBailout4
    //! RUN           force_jit: true, entry: "memmove_bailout.TestBailout::refArray"
    //! METHOD        "memmove_bailout.TestBailout::refArray"
    //! PASS_AFTER    "LoopIdioms"
    //! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/
    static refArray(): boolean {
        let src = new Array<int>(6, 7, 8);
        let dst = new Array<int>(1, 2, 3, 4, 5);
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return check(dst, [6, 7, 8, 4, 5]);
    }

    //! CHECKER       TestBailout5
    //! RUN           force_jit: true, entry: "memmove_bailout.TestBailout::differentTypes"
    //! METHOD        "memmove_bailout.TestBailout::differentTypes"
    //! PASS_AFTER    "LoopIdioms"
    //! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/
    static differentTypes(): boolean {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<long> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return check(dst, [6, 7, 8, 4, 5]);
    }
}
