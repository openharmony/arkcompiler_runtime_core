/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

//! CHECKER           JIT - Check Array actual length range for BoundsCheck.
//! SKIP_IF           @architecture == "arm32"
//! RUN               force_jit: true, options: "--compiler-regex=.*func[0-9]+ --compiler-inlining=true", entry: "checks_elimination_actual_length.ETSGLOBAL::main"
//! METHOD            "checks_elimination_actual_length.Derived::func1"
//! PASS_AFTER        "Inline"
//! INST              /LenArray/
//! INST_NEXT         /BoundsCheck/
//! PASS_AFTER_NEXT   "ChecksElimination"
//! INST_NOT          /BoundsCheck/
//!
//! METHOD            "checks_elimination_actual_length.Derived::func2"
//! PASS_AFTER        "Inline"
//! INST              /LenArray/
//! INST_NEXT         /BoundsCheck/
//! PASS_AFTER_NEXT   "ChecksElimination"
//! INST_NOT          /BoundsCheck/
//!
//! METHOD            "checks_elimination_actual_length.Derived::func3"
//! PASS_AFTER        "Inline"
//! INST              /LenArray/
//! INST_NEXT         /BoundsCheck/
//! PASS_AFTER_NEXT   "ChecksElimination"
//! INST_NOT          /BoundsCheck/
//!
//! METHOD            "checks_elimination_actual_length.Derived::func4"
//! PASS_AFTER        "Inline"
//! INST              /LenArray/
//! INST_NEXT         /BoundsCheck/
//! PASS_AFTER_NEXT   "ChecksElimination"
//! INST_NOT          /BoundsCheck/
//!
//! EVENT             /Compilation,checks_elimination_actual_length.Derived::func5,.*,COMPILED/
//! EVENT_NEXT        /Exception,.*,BOUND_CHECK/
//! METHOD            "checks_elimination_actual_length.Derived::func5"
//! PASS_AFTER        "Inline"
//! PASS_AFTER_NEXT   "ChecksElimination"
//! INST              /BoundsCheck/
//!
//! EVENT             /Compilation,checks_elimination_actual_length.Derived::func6,.*,COMPILED/
//! EVENT_NEXT        /Exception,.*,BOUND_CHECK/
//! METHOD            "checks_elimination_actual_length.Derived::func6"
//! PASS_AFTER        "Inline"
//! PASS_AFTER_NEXT   "ChecksElimination"
//! INST              /BoundsCheck/
//!
//! EVENT             /Compilation,checks_elimination_actual_length.Derived::func7,.*,COMPILED/
//! EVENT_NEXT        /Exception,.*,BOUND_CHECK/
//! METHOD            "checks_elimination_actual_length.Derived::func7"
//! PASS_AFTER        "Inline"
//! PASS_AFTER_NEXT   "ChecksElimination"
//! INST              /BoundsCheck/
//!
//! EVENT             /Compilation,checks_elimination_actual_length.Derived::func8,.*,COMPILED/
//! EVENT_NEXT        /Exception,.*,BOUND_CHECK/
//! METHOD            "checks_elimination_actual_length.Derived::func8"
//! PASS_AFTER        "Inline"
//! PASS_AFTER_NEXT   "ChecksElimination"
//! INST              /BoundsCheck/
//!
//! METHOD            "checks_elimination_actual_length.Derived::func9"
//! PASS_AFTER        "Inline"
//! PASS_AFTER_NEXT   "ChecksElimination"
//! INST              /BoundsCheck/
//!
//! METHOD            "checks_elimination_actual_length.Derived::func10"
//! PASS_AFTER        "Inline"
//! PASS_AFTER_NEXT   "ChecksElimination"
//! INST              /BoundsCheck/
//!
//! METHOD            "checks_elimination_actual_length.ETSGLOBAL::func11"
//! PASS_AFTER        "Inline"
//! INST_NEXT         /BoundsCheck/
//! PASS_AFTER_NEXT   "ChecksElimination"
//! INST_NOT          /BoundsCheck/
//!
//! METHOD            "checks_elimination_actual_length.ETSGLOBAL::func12"
//! PASS_AFTER        "Inline"
//! INST_NEXT         /BoundsCheck/
//! PASS_AFTER_NEXT   "ChecksElimination"
//! INST_NOT          /BoundsCheck/
class Derived extends Array<int> {
    public constructor(len : number) {
        super(len)
    }

    // --------------------- Positive tests ---------------------- //

    public func1() : int {
        let sum : int = 0
        let length = this.length as int
        for (let i : int = 0; i < length; i++) {
            sum += this.$_get_unsafe(i)
        }
        return sum
    }

    public func2() : int {
        let sum : int = 0
        let length = this.length as int
        for (let i : int = length - 1; i >= 0; i--) {
            sum += this.$_get_unsafe(i)
        }
        return sum
    }

    public func3() : int {
        let sum : int = 0
        let length = this.length as int
        for (let i : int = 0; i < length / 2; i++) {
            sum += this.$_get_unsafe(i)
        }
        return sum
    }

    public func4() : int {
        let sum : int = 0
        let length = this.length as int
        for (let i : int = 0; i < length; i++) {
            sum += this.$_get_unsafe(length - i - 1)
        }
        return sum
    }

    // --------------------- Negative tests ---------------------- //

    public func5() : int {
        let has_error : boolean = false
        let sum : int = 0
        let length = this.length as int
        for (let i : int = 0; i < length + 1; i++) {
            try {
                sum += this.$_get_unsafe(i)
            } catch (e: ArrayIndexOutOfBoundsError) {
                has_error = true
            }
        }
        assertTrue(has_error)
        return sum
    }

    public func6() : int {
        let has_error : boolean = false
        let sum : int = 0
        let length = this.length as int
        for (let i : int = 0; i < length; i--) {
            try {
                sum += this.$_get_unsafe(i)
            } catch (e: ArrayIndexOutOfBoundsError) {
                has_error = true
                break
            }
        }
        assertTrue(has_error)
        return sum
    }

    public func7() : int {
        let has_error : boolean = false
        let sum : int = 0
        let length = this.length as int
        for (let i : int = length; i >= 0; i--) {
            try {
                sum += this.$_get_unsafe(i)
            } catch (e: ArrayIndexOutOfBoundsError) {
                has_error = true
            }
        }
        assertTrue(has_error)
        return sum
    }

    public func8() : int {
        let has_error : boolean = false
        let sum : int = 0
        let length = this.length as int
        for (let i : int = -1; i < length; i++) {
            try {
                sum += this.$_get_unsafe(i)
            } catch (e: ArrayIndexOutOfBoundsError) {
                has_error = true
            }
        }
        assertTrue(has_error)
        return sum
    }

    public func9() : int {
        let sum : int = 0
        for (let i : int = 0; i < (this.length as int); i++) {
            this.length = this.length - 1
            sum += this[i]
        }
        return sum
    }

    public func10() : int {
        let sum : int = 0
        for (let i : int = 0; i < (this.length as int); i++) {
            this.__noinline__setlen(3)
            sum += this[i]
        }
        return sum
    }

    private __noinline__setlen(len : int) : void {
        this.length = len
    }

}

// --------------------- Positive tests ---------------------- //

function func11(arr : Array<int>) : Array<int> {
    return arr.toReversed()
}

function func12(arr : Array<int>) : Array<int> {
    return arr.reverse()
}

function main() {
    let a = new Derived(4)
    for (let i : int = 0; i < 4; i++) {
        a[i] = i
    }
    let a2 = new Derived(4)
    for (let i : int = 0; i < 4; i++) {
        a2[i] = i
    }

    // --------------------- Positive tests ---------------------- //

    assertEQ(a.func1(), 6)
    assertEQ(a.func2(), 6)
    assertEQ(a.func3(), 1)
    assertEQ(a.func4(), 6)

    let b = func11(a as Array<int>)
    b = func12(b)
    for (let i : int = 0; i < 4; i++) {
        assertEQ(b[i], i)
    }

    // --------------------- Negative tests ---------------------- //

    assertEQ(a.func5(), 6)
    assertEQ(a.func6(), 0)
    assertEQ(a.func7(), 6)
    assertEQ(a.func8(), 6)
    assertEQ(a.func9(), 1)
    assertEQ(a2.func10(), 3)
}
