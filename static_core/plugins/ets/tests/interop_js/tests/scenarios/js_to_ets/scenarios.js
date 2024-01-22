/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

exports.standaloneFunctionJs = standaloneFunctionJs;
exports.ClassWithMethodJs = ClassWithMethodJs;
exports.newInterfaceWithMethod = newInterfaceWithMethod;
exports.ClassWithGetterSetter = ClassWithGetterSetter;
exports.lambdaFunction = lambdaFunction;
exports.genericFunction = genericFunction;
exports.ClassToExtend = ClassToExtend;
exports.functionArgTypeAny = functionArgTypeAny;
exports.functionArgTypeUnknown = functionArgTypeUnknown;
exports.functionArgTypeUndefined = functionArgTypeUndefined;
exports.functionArgTypeTuple = functionArgTypeTuple;
exports.functionReturnTypeAny = functionReturnTypeAny;
exports.functionReturnTypeUnknown = functionReturnTypeUnknown;
exports.functionReturnTypeUndefined = functionReturnTypeUndefined;
exports.functionArgTypeCallable = functionArgTypeCallable;
