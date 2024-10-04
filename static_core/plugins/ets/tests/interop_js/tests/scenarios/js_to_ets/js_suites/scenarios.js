/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

const STRING_VALUE = '1';
const INT_VALUE = 1;
const INT_VALUE2 = 2;
const INT_VALUE3 = 3;
const FLOAT_VALUE = 1.0;


function standaloneFunctionJs() {
	return 1;
}

class ClassWithMethodJs {
	methodInClassJs() {
		return 1;
	}
}

class InterfaceWithMethodImpl {
	methodInInterface() {
		return 1;
	}
}

function newInterfaceWithMethod() {
	let implInterfaceWithMethod = new InterfaceWithMethodImpl();
	return implInterfaceWithMethod;
	/* above is transpiled from the following TS code:
    interface InterfaceWithMethod {
      methodInInterface(): int;
    }

    class InterfaceWithMethodImpl implements InterfaceWithMethod {
      methodInInterface(): int {
        return 1;
      }
    }

    function newInterfaceWithMethod(): InterfaceWithMethod {
      let implInterfaceWithMethod: InterfaceWithMethod = new InterfaceWithMethodImpl();
      return implInterfaceWithMethod;
    }
  */
}

class ClassWithGetterSetter {
	_value = 1;

	get value() {
		return this._value;
	}

	set value(theValue) {
		this._value = theValue;
	}
}

let lambdaFunction = () => {
	return 1;
};

function genericFunction(arg) {
	return arg;
}

function genericTypeParameter(arg) {
	return arg.toString();
}

function genericTypeReturnValue(arg) {
	return arg;
}

class ClassToExtend {
	value() {
		return 1;
	}
}

// NOTE(oignatenko) return and arg types any, unknown, undefined need real TS because transpiling cuts off
//   important details. Have a look at declgen_ets2ts
function functionArgTypeAny(arg) {
	return arg; // transpiled from Typescript code: functionArgTypeAny(arg: any)
}

function functionArgTypeUnknown(arg) {
	return arg; // transpiled from Typescript code: functionArgTypeUnknown(arg: unknown)
}

function functionArgTypeUndefined(arg) {
	return arg; // transpiled from Typescript code: functionArgTypeUndefined(arg: undefined)
}

function functionArgTypeTuple(arg) {
	return arg[0]; // transpiled from Typescript code: functionArgTypeTuple(arg: [number, string]): number
}

function functionReturnTypeAny() {
	let value = 1;
	return value; // transpiled from Typescript code:functionReturnTypeAny(): any
}

function functionReturnTypeUnknown() {
	let value = 1;
	return value; // transpiled from Typescript code: functionReturnTypeUnknown(): unknown
}

function functionReturnTypeUndefined() {
	let value = 1;
	return value; // transpiled from Typescript code: functionReturnTypeUndefined(): undefined
}

function functionArgTypeCallable(functionToCall) {
	return functionToCall();
	// transpiled from Typescript code: unctionArgTypeCallable(functionToCall: () => number): number
}

function functionDefaultParameterFunction(arg1 = INT_VALUE, arg2 = INT_VALUE2, arg3 = INT_VALUE3) {
	let value = 1;
	return value;
	// transpiled from Typescript code:
	// functionDefaultParameterFunction(
	//	arg1: JSValue = INT_VALUE,
	//	arg2: JSValue = INT_VALUE2,
	//	arg3: JSValue = INT_VALUE3): int
}

function functionDefaultIntParameterFunction(arg = INT_VALUE) {
	return arg;
	// transpiled from Typescript code: function defaultFloatParameterFunction(arg: JSValue = INT_VALUE): JSValue
}

function functionDefaultStringParameterFunction(arg = STRING_VALUE) {
	return arg;
	// transpiled from Typescript code: functionDefaultStringParameterFunction(arg: JSValue = STRING_VALUE): JSValue{
}

function functionDefaultFloatParameterFunction(arg = FLOAT_VALUE) {
	return arg;
	// transpiled from Typescript code: function defaultFloatParameterFunction(arg: JSValue = FLOAT_VALUE): JSValue{
}

function functionArgTypeOptionalPrimitive(arg) {
	if (typeof arg !== 'undefined') {
		return arg;
	}
	return INT_VALUE;
}

function functionArgTypePrimitive(arg) {
	return arg;
}

function functionReturnTypePrimitive() {
	return true;
}

function optionalCall(x = 123, y = 130, z = 1) {
	return x + y + z;
}

function singleRequired(z, x = 123, y = 123) {
	return x + y + z;
}

function jsSumRestParams(...args) {
	let sum = 0;
	args.forEach((n) => (sum += n));
	return sum;
}

function jsMultiplyArgBySumRestParams(arg0, ...args) {
	let sum = 0;
	args.forEach((n) => (sum += n));
	return sum * arg0;
}

function jsMultiplySumArgsBySumRestParams(arg0, arg1, ...args) {
	let sum = 0;
	args.forEach((n) => (sum += n));
	return sum * (arg0 + arg1);
}

function jsConcatStringsRestParams(...args) {
	let str = '';
	args.forEach((s) => (str += s));
	return str;
}

class InterfaceWithUnionImpl {
	methodInInterface(arg) {
		return functionReturnTypeUnion(arg);
	}
}

function newInterfaceWithUnion() {
	return new InterfaceWithUnionImpl();
}

function functionArgTypeUnion(arg) {
	switch (typeof arg) {
		case 'number':
			return 0;
		case 'string':
			return 1;
		default:
			return -1;
	}
}

function functionReturnTypeUnion(arg) {
	if (arg === 0) {
		return INT_VALUE;
	}
	return STRING_VALUE;
}

class UnionTestClassJs {
	methodArgTypeUnion(arg) {
		return functionArgTypeUnion(arg);
	}
	methodReturnTypeUnion(arg) {
		return functionReturnTypeUnion(arg);
	}
}

class ClassWithStaticMethod {
	static staticMethod(arg) {
		return arg;
	}
	static staticMethodReturnTypeUnion(arg) {
		return functionReturnTypeUnion(arg);
	}
}
function functionRestParameter(...arg) {
	return arg[0]; // transpiled from Typescript code: function functionRestParameter(...arg: number[])
}

function functionSpreadParameter(arg1, arg2) {
	return arg1 + arg2; // transpiled from Typescript code: function functionSpreadParameter(arg1: number, arg2: number)
}

function functionOverloaded() {}

function functionOverloaded(arg) {}

function functionOverload() {
	let value = 1;
	functionOverloaded();
	functionOverloaded(value);
	return value;
}

function functionCallableReturnValue() {
	const value = () => (x) => Number(x) + 1;
	console.log(value()(4));
	return Function('', 'return 2');
}

function functionArgStringLiteralType(arg) {
	return arg;
	// transpiled from Typescript code: functionArgStringLiteralType(arg: TypeString): TypeString
}

function functionIntersectionTypePrimitive(arg) {
	const ret = arg;
	return ret;
	// transpiled from Typescript code: functionIntersectionTypePrimitive(arg: PrimitiveAB): PrimitiveAB
}

class ClassWithInFieldDeclaration {
    constructor(value) {
        this.value = value;
    }
}

function functionReturnsCompositeType(arg) {
    switch (arg){
      case 0:
        return STRING_VALUE
      case 1:
        return FLOAT_VALUE
      case 2:
        return INT_VALUE
      default:
        return [ 'arrayvalue1', 'arrayvalue2' ]
    } 
}

exports.standaloneFunctionJs = standaloneFunctionJs;
exports.ClassWithMethodJs = ClassWithMethodJs;
exports.newInterfaceWithMethod = newInterfaceWithMethod;
exports.ClassWithGetterSetter = ClassWithGetterSetter;
exports.lambdaFunction = lambdaFunction;
exports.genericFunction = genericFunction;
exports.genericTypeParameter = genericTypeParameter;
exports.genericTypeReturnValue = genericTypeReturnValue;
exports.ClassToExtend = ClassToExtend;
exports.functionArgTypeAny = functionArgTypeAny;
exports.functionArgTypeUnknown = functionArgTypeUnknown;
exports.functionArgTypeUndefined = functionArgTypeUndefined;
exports.functionArgTypeTuple = functionArgTypeTuple;
exports.functionReturnTypeAny = functionReturnTypeAny;
exports.functionReturnTypeUnknown = functionReturnTypeUnknown;
exports.functionReturnTypeUndefined = functionReturnTypeUndefined;
exports.functionArgTypeCallable = functionArgTypeCallable;
exports.functionDefaultParameterFunction = functionDefaultParameterFunction;
exports.functionDefaultIntParameterFunction = functionDefaultIntParameterFunction;
exports.functionDefaultStringParameterFunction = functionDefaultStringParameterFunction;
exports.functionDefaultFloatParameterFunction = functionDefaultFloatParameterFunction;
exports.functionArgTypeOptionalPrimitive = functionArgTypeOptionalPrimitive;
exports.functionArgTypePrimitive = functionArgTypePrimitive;
exports.functionReturnTypePrimitive = functionReturnTypePrimitive;
exports.optionalCall = optionalCall;
exports.singleRequired = singleRequired;
exports.jsSumRestParams = jsSumRestParams;
exports.jsMultiplyArgBySumRestParams = jsMultiplyArgBySumRestParams;
exports.jsMultiplySumArgsBySumRestParams = jsMultiplySumArgsBySumRestParams;
exports.jsConcatStringsRestParams = jsConcatStringsRestParams;
exports.InterfaceWithUnionImpl = InterfaceWithUnionImpl;
exports.newInterfaceWithUnion = newInterfaceWithUnion;
exports.functionArgTypeUnion = functionArgTypeUnion;
exports.functionReturnTypeUnion = functionReturnTypeUnion;
exports.UnionTestClassJs = UnionTestClassJs;
exports.ClassWithStaticMethod = ClassWithStaticMethod;
exports.functionRestParameter = functionRestParameter;
exports.functionSpreadParameter = functionSpreadParameter;
exports.functionOverload = functionOverload;
exports.functionCallableReturnValue = functionCallableReturnValue;
exports.functionArgStringLiteralType = functionArgStringLiteralType;
exports.functionIntersectionTypePrimitive = functionIntersectionTypePrimitive;
exports.ClassWithInFieldDeclaration = ClassWithInFieldDeclaration;
exports.functionReturnsCompositeType = functionReturnsCompositeType;
