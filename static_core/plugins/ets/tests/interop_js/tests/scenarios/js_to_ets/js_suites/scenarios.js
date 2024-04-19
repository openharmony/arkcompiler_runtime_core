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
  // function default_parameter_function(arg1: JSValue = INT_VALUE, arg2: JSValue = INT_VALUE2, arg3: JSValue = INT_VALUE3): int
}

function functionDefaultIntParameterFunction(arg = INT_VALUE) {
  return arg;
  // transpiled from Typescript code: function default_float_parameter_function(arg: JSValue = INT_VALUE): JSValue
}

function functionDefaultStringParameterFunction(arg = STRING_VALUE) {
  return arg;
  // transpiled from Typescript code: function default_string_parameter_function(arg: JSValue = STRING_VALUE): JSValue{
}

function functionDefaultFloatParameterFunction(arg = FLOAT_VALUE) {
  return arg;
  // transpiled from Typescript code: function default_float_parameter_function(arg: JSValue = FLOAT_VALUE): JSValue{
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


function optional_call(x=123, y=130, z=1) {
  return x + y + z;
}

function single_required(z, x=123, y=123) {
  return x + y + z;
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
exports.optional_call = optional_call;
exports.single_required = single_required;
