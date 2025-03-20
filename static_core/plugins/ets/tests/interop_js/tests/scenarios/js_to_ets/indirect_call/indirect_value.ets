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
import * as js from "indirect_js"

function Test_indirect_call_type_value_number_call(): boolean {
    const EXPECTED = 2;

    let result = js.indirectCallTypeByValueNumber.call(null, 1) as int;
    assertTrue(result == EXPECTED);

    return true;
}

function Test_indirect_call_type_value_number_apply(): boolean {
    const EXPECTED = 2;

    let result = js.indirectCallTypeByValueNumber.apply(null, [1] as FixedArray<int>) as int;
    assertTrue(result == EXPECTED);

    return true;
}

function Test_indirect_call_type_value_number_bind_with_arg(): boolean {
    const EXPECTED = 2;

    let boundWithArg = js.indirectCallTypeByValueNumber.bind(null, 1);
    assertTrue(boundWithArg() as int == EXPECTED);

    return true;
}

function Test_indirect_call_type_value_number_bind_without_arg(): boolean {
    const EXPECTED = 2;

    let boundWithoutArg = js.indirectCallTypeByValueNumber.bind(null);
    assertTrue(boundWithoutArg(1) as int == EXPECTED);

    return true;
}

function Test_indirect_call_type_value_string_call(): boolean {
    const ARG = 'foo'
    const EXPECTED = ''.concat(ARG, ARG);

    let result = js.indirectCallTypeByValueString.call(null, ARG) as string;
    assertTrue(result == EXPECTED);

    return true;
}

function Test_indirect_call_type_value_string_apply(): boolean {
    const ARG = 'foo'
    const EXPECTED = ''.concat(ARG, ARG);

    let result = js.indirectCallTypeByValueString.apply(null, [ARG] as FixedArray<string>) as string;
    assertTrue(result == EXPECTED);

    return true;
}

function Test_indirect_call_type_value_string_bind_with_arg(): boolean {
    const ARG = 'foo'
    const EXPECTED = ''.concat(ARG, ARG);

    let boundWithArg = js.indirectCallTypeByValueString.bind(null, ARG);
    assertTrue(boundWithArg() as string == EXPECTED);

    return true;
}

function Test_indirect_call_type_value_string_bind_without_arg(): boolean {
    const ARG = 'foo'
    const EXPECTED = ''.concat(ARG, ARG);

    let boundWithoutArg = js.indirectCallTypeByValueString.bind(null);
    assertTrue(boundWithoutArg(ARG) as string == EXPECTED);

    return true;
}

function Test_indirect_call_type_value_boolean_call(): boolean {
    const ARG = true;
    const EXPECTED = !ARG;

    let result = js.indirectCallTypeByValueBoolean.call(null, ARG) as boolean;
    assertTrue(result == EXPECTED);

    return true;
}

function Test_indirect_call_type_value_boolean_apply(): boolean {
    const ARG = true;
    const EXPECTED = !ARG;

    let result = js.indirectCallTypeByValueBoolean.apply(null, [ARG] as FixedArray<boolean>) as boolean;
    assertTrue(result == EXPECTED);

    return true;
}

function Test_indirect_call_type_value_boolean_bind_with_arg(): boolean {
    const ARG = true;
    const EXPECTED = !ARG;

    let boundWithArg = js.indirectCallTypeByValueBoolean.bind(null, ARG);
    assertTrue(boundWithArg() as boolean == EXPECTED);

    return true;
}

function Test_indirect_call_type_value_boolean_bind_without_arg(): boolean {
    const ARG = true;
    const EXPECTED = !ARG;

    let boundWithoutArg = js.indirectCallTypeByValueBoolean.bind(null);
    assertTrue(boundWithoutArg(ARG) as boolean == EXPECTED);

    return true;
}
