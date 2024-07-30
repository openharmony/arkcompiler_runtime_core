/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import * as js from "pure_js_conflict"

function Test_function_arg_type_conflict_array(): boolean {
    const array:Array<Object> = js.functionArgTypeConflictArray() as Array<Object>;
    const EXPECTED_LENGTH = 2;
    const EXPECTED_INT = 123;
    const EXPECTED_STRING = 'foo';

    assert(array.length == EXPECTED_LENGTH);

    return (array.at(0) as Number == EXPECTED_INT) && (array.at(1) as string == EXPECTED_STRING);
}

function Test_function_arg_type_conflict_arraybuffer(): boolean {
    const buffer = js.functionArgTypeConflictArrayBuffer() as ArrayBuffer;
    const view = new Int32Array(buffer);
    const EXPECTED_INT = 123;

    return view.at(0) == EXPECTED_INT;
}

function Test_function_arg_type_conflict_boolean(): boolean {
    const value = js.functionArgTypeConflictBoolean() as Boolean;
    return value == true;
}

function Test_function_arg_type_conflict_dataview(): boolean {
    const view = js.functionArgTypeConflictDataView() as DataView;
    const EXPECTED_INT = 123;

    return view.getInt32(0) == EXPECTED_INT;
}

function Test_function_arg_type_conflict_date(): boolean {
    const date = js.functionArgTypeConflictDate() as Date;
    return date.valueOf() == 1;
}

function Test_function_arg_type_conflict_error(): boolean {
    const error = js.functionArgTypeConflictError() as Error;
    const EXPECTED_STRING = 'foo';

    return error.message == EXPECTED_STRING;
}

function Test_function_arg_type_conflict_map(): boolean {
    const dataMap = js.functionArgTypeConflictMap() as Map<string, string>;
    const KEY = 'foo';
    const EXPECTED_VALUE = 'bar';

    return dataMap.get(KEY) == EXPECTED_VALUE;
}

function Test_function_arg_type_conflict_object(): boolean {
    const object = js.functionArgTypeConflictObject() as Object;
    return typeof object == 'object' && object instanceof Object;
}

function Test_function_arg_type_conflict_string(): boolean {
    const value = js.functionArgTypeConflictString() as string;
    const EXPECTED_STRING = 'foo';

    return value == EXPECTED_STRING;
}
