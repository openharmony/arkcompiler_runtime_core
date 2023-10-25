/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

//! CHECKER      AOT IR Builder, check StringEquals intrinsic
//! RUN_PAOC     options: "--compiler-regex='.*equals_.*' --compiler-inlining=false"
//!
//! METHOD       "ETSGLOBAL::equals_str_obj"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.StdCoreStringEquals/
//! INST_NEXT_NOT /Intrinsic/
//!
//! METHOD       "ETSGLOBAL::equals_obj_obj"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     /Intrinsic/
//!  
//! METHOD       "ETSGLOBAL::equals_null_str"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     /Intrinsic.StdCoreStringEquals/
//!
//! METHOD       "ETSGLOBAL::equals_str_null"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.StdCoreStringEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.StdCoreStringEquals/
//!
//! METHOD       "ETSGLOBAL::not_equals_null_str"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     /Intrinsic.StdCoreStringEquals/
//!
//! METHOD       "ETSGLOBAL::not_equals_str_null"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.StdCoreStringEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.StdCoreStringEquals/
//!
//! RUN          entry: "ETSGLOBAL::main"

function equals_str_obj(lhs: String | null, rhs: Object | null): boolean {
    return lhs == rhs;
}

function equals_obj_obj(lhs: Object | null, rhs: Object | null): boolean {
    return lhs == rhs;
}

function equals_null_str(str: String | null): boolean {
    return null == str;
}

function not_equals_null_str(str: String | null): boolean {
    return null != str;
}

function equals_str_null(str: String | null): boolean {
    return str == null;
}

function not_equals_str_null(str: String | null): boolean {
    return str != null;
}

class A {}

function main() {
    let str = "abc";
    let obj = new A();

    assert equals_str_obj(str, str);
    assert !equals_str_obj("abcd", "abce");
    assert !equals_str_obj(str, obj);
    assert !equals_str_obj(str, null);
    assert equals_str_obj(null, null);

    assert equals_obj_obj(str, str);
    assert !equals_obj_obj(str, obj);
    assert !equals_obj_obj(str, null);
    assert equals_obj_obj(null, null);

    assert !equals_null_str(str);
    assert not_equals_null_str(str);
    assert !equals_str_null(str);
    assert not_equals_str_null(str);

    assert equals_null_str(null);
    assert !not_equals_null_str(null);
    assert equals_str_null(null);
    assert !not_equals_str_null(null);
}
