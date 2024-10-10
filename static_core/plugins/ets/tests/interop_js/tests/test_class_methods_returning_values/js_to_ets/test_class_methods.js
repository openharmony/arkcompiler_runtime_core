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
exports.returnTestClassInstance =
	exports.ReturnTestClass =
	exports.returnInterface =
	exports.returnClass =
	exports.TestClass =
	exports.returnUnion =
	exports.returnNullSubsetByValue =
	exports.returnUndefinedSubsetByValue =
	exports.returnBoolSubsetByValue =
	exports.returnIntSubsetByValue =
	exports.returnLongIntSubsetByValue =
	exports.returnStringSubsetByValue =
	exports.returnMapSubsetByRef =
	exports.returnStringSubsetByRef =
	exports.isTTupleTS =
	exports.returnTuple =
	exports.returnSet =
	exports.returnMap =
	exports.returnsBigNLiteral =
	exports.returnBoolLiteral =
	exports.returnIntLiteral =
	exports.returnStrLiteral =
	exports.returnSetAsAny =
	exports.returnMapAsAny =
	exports.returnNullAsAny =
	exports.returnUndefinedAsAny =
	exports.returnBooleanAsAny =
	exports.returnBigIntegerAsAny =
	exports.returnStringAsAny =
	exports.returnIntegerAsAny =
		void 0;
const TEST_STRING = 'This is a test string';
const TEST_LONG_INT = 9007199254740991;
const TEST_INT = 100;
const TEST_BIG_INT = 10000000n;
const TEST_BOOLEAN = true;
const TEST_MAP = new Map([[TEST_STRING.toUpperCase(), TEST_STRING.toLowerCase()]]);
const TEST_SET = new Set([TEST_STRING.toUpperCase(), TEST_STRING.toLowerCase()]);
const TEST_CHANGEABLE_MAP = new Map([['1', 'Test']]);
const returnIntegerAsAny = function returnInteger() {
	return TEST_INT;
};

exports.returnIntegerAsAny = returnIntegerAsAny;

const returnStringAsAny = function returnString() {
	return TEST_STRING;
};

exports.returnStringAsAny = returnStringAsAny;

const returnBigIntegerAsAny = function returnBigInteger() {
	return TEST_BIG_INT;
};

exports.returnBigIntegerAsAny = returnBigIntegerAsAny;

const returnBooleanAsAny = function returnBoolean() {
	return TEST_BOOLEAN;
};

exports.returnBooleanAsAny = returnBooleanAsAny;

const returnUndefinedAsAny = function returnUndefined() {
	return;
};

exports.returnUndefinedAsAny = returnUndefinedAsAny;

const returnNullAsAny = function returnNull() {
	return null;
};

exports.returnNullAsAny = returnNullAsAny;

const returnMapAsAny = function returnMap() {
	return TEST_MAP;
};

exports.returnMapAsAny = returnMapAsAny;

const returnSetAsAny = function returnSet() {
	return TEST_SET;
};

exports.returnSetAsAny = returnSetAsAny;

const returnStrLiteral = function returnStrLiteral() {
	const literalValue = 'Test';
	return literalValue;
};

exports.returnStrLiteral = returnStrLiteral;

const returnIntLiteral = function returnIntLiteral() {
	const literalValue = 1;
	return literalValue;
};

exports.returnIntLiteral = returnIntLiteral;

const returnBoolLiteral = function returnBoolLiteral() {
	const literalValue = false;
	return literalValue;
};

exports.returnBoolLiteral = returnBoolLiteral;

const returnsBigNLiteral = function returnsBigNLiteral() {
	const literalValue = 100n;
	return literalValue;
};

exports.returnsBigNLiteral = returnsBigNLiteral;

const returnMap = function returnMap() {
	return TEST_MAP;
};

exports.returnMap = returnMap;

const returnSet = function returnSet() {
	return TEST_SET;
};

exports.returnSet = returnSet;

const returnTuple = function returnTuple() {
	return [TEST_STRING, TEST_LONG_INT, TEST_BOOLEAN];
};

exports.returnTuple = returnTuple;

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

const returnStringSubsetByRef = function returnStringSubsetByRef() {
	return TEST_STRING;
};

exports.returnStringSubsetByRef = returnStringSubsetByRef;

const returnMapSubsetByRef = function returnMapSubsetByRef() {
	return TEST_CHANGEABLE_MAP;
};

exports.returnMapSubsetByRef = returnMapSubsetByRef;

const returnStringSubsetByValue = function returnString() {
	return TEST_STRING;
};

exports.returnStringSubsetByValue = returnStringSubsetByValue;

const returnLongIntSubsetByValue = function returnInteger() {
	return TEST_LONG_INT;
};

exports.returnLongIntSubsetByValue = returnLongIntSubsetByValue;

const returnIntSubsetByValue = function returnInteger() {
	return TEST_INT;
};

exports.returnIntSubsetByValue = returnIntSubsetByValue;

const returnBoolSubsetByValue = function returnBoolean() {
	return TEST_BOOLEAN;
};

exports.returnBoolSubsetByValue = returnBoolSubsetByValue;

const returnUndefinedSubsetByValue = function returnUndefined() {
	return;
};

exports.returnUndefinedSubsetByValue = returnUndefinedSubsetByValue;

const returnNullSubsetByValue = function returnNull() {
	return null;
};

exports.returnNullSubsetByValue = returnNullSubsetByValue;
const returnUnion = function returnUnion(isStr) {
	if (isStr) {
		return '1000';
	}
	return 1000;
};

exports.returnUnion = returnUnion;

class TestClass {
	constructor(testNumber, testString) {
		this.testNumber = testNumber;
		this.testString = testString;
	}
	testFunction() {
		return this.testNumber;
	}
	static isTestClass(instance) {
		return instance instanceof TestClass;
	}
}

exports.TestClass = TestClass;

const returnClass = function returnClass() {
	const testClass = new TestClass(1, 'Test');
	return testClass;
};

exports.returnClass = returnClass;

const returnInterface = function returnInterface() {
	const testInterface = {
		testNumber: 100,
		testString: 'Test',
	};
	return testInterface;
};

exports.returnInterface = returnInterface;

class ReturnTestClass {
	constructor() {
		this.returnIntegerAsAny = exports.returnIntegerAsAny.bind(this);
		this.returnStringAsAny = exports.returnStringAsAny.bind(this);
		this.returnBigIntegerAsAny = exports.returnBigIntegerAsAny.bind(this);
		this.returnBooleanAsAny = exports.returnBooleanAsAny.bind(this);
		this.returnUndefinedAsAny = exports.returnUndefinedAsAny.bind(this);
		this.returnNullAsAny = exports.returnNullAsAny.bind(this);
		this.returnMapAsAny = exports.returnMapAsAny.bind(this);
		this.returnSetAsAny = exports.returnSetAsAny.bind(this);
		this.returnStrLiteral = exports.returnStrLiteral.bind(this);
		this.returnIntLiteral = exports.returnIntLiteral.bind(this);
		this.returnBoolLiteral = exports.returnBoolLiteral.bind(this);
		this.returnsBigNLiteral = exports.returnsBigNLiteral.bind(this);
		this.returnMap = exports.returnMap.bind(this);
		this.returnSet = exports.returnSet.bind(this);
		this.returnTuple = exports.returnTuple.bind(this);
		this.returnStringSubsetByRef = exports.returnStringSubsetByRef.bind(this);
		this.returnMapSubsetByRef = exports.returnMapSubsetByRef.bind(this);
		this.returnStringSubsetByValue = exports.returnStringSubsetByValue.bind(this);
		this.returnIntSubsetByValue = exports.returnIntSubsetByValue.bind(this);
		this.returnLongIntSubsetByValue = exports.returnLongIntSubsetByValue.bind(this);
		this.returnBoolSubsetByValue = exports.returnBoolSubsetByValue.bind(this);
		this.returnUndefinedSubsetByValue = exports.returnUndefinedSubsetByValue.bind(this);
		this.returnNullSubsetByValue = exports.returnNullSubsetByValue.bind(this);
		this.returnUnion = exports.returnUnion.bind(this);
		this.returnClass = exports.returnClass.bind(this);
		this.returnInterface = exports.returnInterface.bind(this);
	}
}

exports.ReturnTestClass = ReturnTestClass;

exports.returnTestClassInstance = new ReturnTestClass();

const testAny = () => {
	console.log(
		[
			exports.returnIntegerAsAny,
			exports.returnStringAsAny,
			exports.returnBigIntegerAsAny,
			exports.returnBooleanAsAny,
			exports.returnUndefinedAsAny,
			exports.returnNullAsAny,
			exports.returnMapAsAny,
			exports.returnSetAsAny,
		].map((x) => x())
	);
};

const testLiterals = () => {
	console.log([exports.returnStrLiteral, exports.returnIntLiteral, exports.returnBoolLiteral, exports.returnsBigNLiteral].map((x) => x()));
};

const testExtraSet = () => {
	console.log([exports.returnMap, exports.returnSet, exports.returnTuple].map((x) => x()));
};

const testSubset = () => {
	console.log(
		[
			exports.returnStringSubsetByRef,
			exports.returnMapSubsetByRef,
			exports.returnStringSubsetByValue,
			exports.returnIntSubsetByValue,
			exports.returnLongIntSubsetByValue,
			exports.returnBoolSubsetByValue,
			exports.returnUndefinedSubsetByValue,
			exports.returnNullSubsetByValue,
		].map((x) => x())
	);
};

const testUnion = () => {
	console.log((0, exports.returnUnion)(true), (0, exports.returnUnion)(false));
};

const testClassInterface = () => {
	console.log([exports.returnClass, exports.returnInterface].map((x) => x()));
};

testAny();
testLiterals();
testExtraSet();
testSubset();
testUnion();
testClassInterface();
