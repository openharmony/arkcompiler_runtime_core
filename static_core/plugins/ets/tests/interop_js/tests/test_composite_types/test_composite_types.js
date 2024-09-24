'use strict';
/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
Object.defineProperty(exports, '__esModule', { value: true });
exports.compositeTypesTestClassInstance =
	exports.CompositeTypesTestClass =
	exports.returnInterface =
	exports.returnRecord =
	exports.returnNumArray =
	exports.returnStrArray =
	exports.returnTuple =
	exports.isTTupleTS =
		void 0;

const TEST_STRING = 'This is a test string';
const TEST_INT = 100;
const TEST_BOOLEAN = true;
const isTTupleTS = (testedVar) => {
	return (
		Array.isArray(testedVar) &&
		testedVar.length === 3 &&
		typeof testedVar[0] === 'string' &&
		typeof testedVar[1] === 'number' &&
		typeof testedVar[2] === 'boolean'
	);
};
exports.isTTupleTS = isTTupleTS;
const testTuple = [TEST_STRING, TEST_INT, TEST_BOOLEAN];
const returnTuple = function returnTuple() {
	return testTuple;
};
exports.returnTuple = returnTuple;
const returnStrArray = function returnArray() {
	const testArray = ['One', 'Two', 'Three'];
	return testArray;
};
exports.returnStrArray = returnStrArray;
const returnNumArray = function returnArrayNum() {
	const testArray = [1, 2, 3];
	return testArray;
};
exports.returnNumArray = returnNumArray;
const returnRecord = function returnRecord() {
	const testRecord = {
		one: 1,
		two: 2,
		three: 3,
	};
	return testRecord;
};
exports.returnRecord = returnRecord;
const returnInterface = function returnInterface(returnThreeDimensioned) {
	const testTwoDimensioned = {
		width: 100,
		length: 40,
	};
	const testThreeDimensioned = {
		width: 100,
		length: 50,
		height: 25,
	};
	if (returnThreeDimensioned) {
		return testThreeDimensioned;
	}
	return testTwoDimensioned;
};
exports.returnInterface = returnInterface;
class CompositeTypesTestClass {
	constructor() {
		this.returnTuple = exports.returnTuple.bind(this);
		this.returnStrArray = exports.returnStrArray.bind(this);
		this.returnNumArray = exports.returnNumArray.bind(this);
		this.returnRecord = exports.returnRecord.bind(this);
		this.returnInterface = exports.returnInterface.bind(this);
	}
}
exports.CompositeTypesTestClass = CompositeTypesTestClass;
exports.compositeTypesTestClassInstance = new CompositeTypesTestClass();
const test = () => {
	console.log([exports.returnTuple, exports.returnStrArray, exports.returnNumArray, exports.returnRecord].map((x) => x()));
};
const testInterface = () => {
	console.log((0, exports.returnInterface)(true), (0, exports.returnInterface)(false));
};
test();
testInterface();
