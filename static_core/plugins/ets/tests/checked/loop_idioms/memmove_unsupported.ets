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

let GLOBAL_OFFS: int = 1;

class TestUnsupported {
    //! CHECKER       TestUnsupported1
    //! RUN           force_jit: true, entry: "memmove_unsupported.TestUnsupported::noAlias_underIf"
    //! METHOD        "memmove_unsupported.TestUnsupported::noAlias_underIf"
    //! PASS_AFTER    "LoopIdioms"
    //! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/
    static noAlias_underIf(): boolean {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        return check(TestUnsupported.__noinline__noAlias_underIf(src, dst), [6, 7, 8, 4, 5]);
    }

    static __noinline__noAlias_underIf(src: FixedArray<int>, dst: FixedArray<int>): FixedArray<int> {
        if (src != dst) {
            for (let i: int = 0; i < src.length; i++) {
                dst[i] = src[i];
            }
        }
        return dst;
    }

    //! CHECKER       TestUnsupported2
    //! RUN           force_jit: true, entry: "memmove_unsupported.TestUnsupported::patternMissmatch_boundscheck"
    //! METHOD        "memmove_unsupported.TestUnsupported::patternMissmatch_boundscheck"
    //! PASS_AFTER    "LoopIdioms"
    //! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/
    static patternMissmatch_boundscheck(): boolean {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        let x = GLOBAL_OFFS;
        for (let i: int = 0; i < src.length; i++) {
            dst[i + GLOBAL_OFFS] = src[i];
        }
        return check(dst, [1, 6, 7, 8, 5]);
    }

    //! CHECKER       TestUnsupported3
    //! RUN           force_jit: true, entry: "memmove_unsupported.TestUnsupported::subIdx"
    //! METHOD        "memmove_unsupported.TestUnsupported::subIdx"
    //! PASS_AFTER    "LoopIdioms"
    //! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/
    static subIdx(): boolean {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        let x = GLOBAL_OFFS;
        for (let i: int = src.length; i > 0; i--) {
            dst[dst.length - i] = src[src.length - i];
        }
        return check(dst, [1, 2, 6, 7, 8]);
    }
};

