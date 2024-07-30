/**
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
'use strict';

function functionArgTypeConflictArray() {
	const SAMPLE_INT = 123;
	const SAMPLE_STRING = 'foo';

	const array = Array.of(SAMPLE_INT, SAMPLE_STRING);
	return array;
}

function functionArgTypeConflictArrayBuffer() {
	const BUFFER_SIZE = 8;
	const VALUE_AS_INT32 = 123;

	const buffer = new ArrayBuffer(BUFFER_SIZE);
	const view = new Int32Array(buffer);

	view.set([VALUE_AS_INT32], 0);

	return buffer;
}

function functionArgTypeConflictBoolean() {
	return new Boolean(true);
}

function functionArgTypeConflictDataView() {
	const BUFFER_SIZE = 8;
	const VALUE_AS_INT32 = 123;

	const buffer = new ArrayBuffer(BUFFER_SIZE);
	const view = new DataView(buffer);
	view.setInt32(0, VALUE_AS_INT32);

	return view;
}

function functionArgTypeConflictDate() {
	return new Date(1);
}

function functionArgTypeConflictError() {
	const SAMPLE_STRING = 'foo';

	return new Error(SAMPLE_STRING);
}

function functionArgTypeConflictMap() {
	const dataMap = new Map();
	const KEY = 'foo';
	const VALUE = 'bar';

	dataMap.set(KEY, VALUE);

	return dataMap;
}

function functionArgTypeConflictObject() {
	return new Object();
}

function functionArgTypeConflictString() {
	const SAMPLE_STRING = 'foo';
	return SAMPLE_STRING;
}

exports.functionArgTypeConflictArray = functionArgTypeConflictArray;
exports.functionArgTypeConflictArrayBuffer = functionArgTypeConflictArrayBuffer;
exports.functionArgTypeConflictBoolean = functionArgTypeConflictBoolean;
exports.functionArgTypeConflictDataView = functionArgTypeConflictDataView;
exports.functionArgTypeConflictDate = functionArgTypeConflictDate;
exports.functionArgTypeConflictError = functionArgTypeConflictError;
exports.functionArgTypeConflictMap = functionArgTypeConflictMap;
exports.functionArgTypeConflictObject = functionArgTypeConflictObject;
exports.functionArgTypeConflictString = functionArgTypeConflictString;
