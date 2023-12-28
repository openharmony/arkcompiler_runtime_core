/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

declare const process: any;
declare const require: any;

const ZERO = 0;
const ONE_TENTH = 0.1;
const TWO_TENTH = 0.2;
const TWENTY_TWO_HUNDREDTH = 0.22;
const THREE_TENTH = 0.3;
const ONE = 1;
const TWO = 2;
const THREE = 3;
const FOUR = 4;
const FIVE = 5;
const TEN = 10;
const FIFTEEN = 15;
const TWENTY = 20;
const PRECISION = 3;

const PANDA_FILES = 'panda-files';
const BOOT_PANDA_FILES = 'boot-panda-files';
const LOAD_RUNTIMES = 'load-runtimes';

(globalThis as any).Panda = require(process.env.MODULE_PATH + '/ets_interop_js_napi.node');

const etsOpts = {
  [PANDA_FILES]: process.env.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
  [BOOT_PANDA_FILES]: `${process.env.ARK_ETS_STDLIB_PATH}:${process.env.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH}`,
  [LOAD_RUNTIMES]: 'ets',
};
const createRes = (globalThis as any).Panda.createRuntime(etsOpts);
if (!createRes) {
  console.log('Cannot create ETS runtime');
  process.exit(1);
}

(globalThis as any).require = require;

import { SumDouble, SumBoxedDouble, SumString, Identity, ForEach, ReplaceFirst } from './lib1';
import { Fields, Methods } from './lib1';
import { Simple, External, Derived, ETSEnum } from './lib1';
import { GenericClass, BaseGeneric } from './lib1';
import type { IGeneric0, IGeneric1 } from './lib1';
import { DynObjWrapper, PromiseWrapper, AsyncIdentity } from './lib1';

function AssertEq<T>(a: T, b: T) {
  console.log(`AssertEq: '${a}' === '${b}'`);
  if (a !== b) {
    throw new Error(`AssertEq failed: '${a}' === '${b}'`);
  }
}

function runTest() {
  testFunctions();
  testFields();
  testMethods();
  testClasses();
  testGenerics();
  testDynObj();
  testPromise();
}

function testFunctions() {
  AssertEq(SumDouble(THREE, TWENTY_TWO_HUNDREDTH).toFixed(PRECISION), (THREE + TWENTY_TWO_HUNDREDTH).toFixed(PRECISION));
  AssertEq(SumBoxedDouble(THREE, TWENTY_TWO_HUNDREDTH).toFixed(PRECISION), (THREE + TWENTY_TWO_HUNDREDTH).toFixed(PRECISION));
  AssertEq(SumString('Hello from ', 'Panda!!'), 'Hello from Panda!!');

  AssertEq(Identity(THREE + TWENTY_TWO_HUNDREDTH).toFixed(PRECISION), (THREE + TWENTY_TWO_HUNDREDTH).toFixed(PRECISION));
  AssertEq(Identity('Panda identity string'), 'Panda identity string');

  // NOTE(ivagin): enable when supported by interop #12808
  if (false) {
    const intArr = [ONE, TWO, THREE];
    ForEach(intArr, (e: number, idx: number) => {
      AssertEq(e, intArr[idx]);
    })
    const intStr = ['1', '2', '3'];
    ForEach(intStr, (e: string, idx: number) => {
      AssertEq(e, intStr[idx]);
    })
  }

  let simpleArray: Simple[] = [new Simple(ZERO), new Simple(ONE), new Simple(TWO)];
  simpleArray.forEach((e: Simple, idx: number) => { AssertEq(e.val, idx); });
  AssertEq(ReplaceFirst(simpleArray, new Simple(THREE))[0].val, THREE);
}

function testFields() {
  const fields = new Fields();
  AssertEq(fields.readonlyBoolean_, true);
  AssertEq(Fields.staticFloat_.toFixed(PRECISION), (ONE + ONE_TENTH).toFixed(PRECISION));
  AssertEq(fields.double_.toFixed(PRECISION), (TWO + TWO_TENTH).toFixed(PRECISION));
  AssertEq(fields.doubleAlias_.toFixed(PRECISION), (THREE + THREE_TENTH).toFixed(PRECISION));
  AssertEq(fields.int_, FOUR);
  // NOTE(ivagin): enable when supported by interop #12808
  if (false) {
    AssertEq(fields.object_ instanceof Object, true);
    AssertEq(fields.rangeError_ instanceof RangeError, true);
    AssertEq(fields.uint8Array_ instanceof Int8Array, true);
    AssertEq(fields.uint16Array_ instanceof Int16Array, true);
    AssertEq(fields.uint32Array_ instanceof Int32Array, true);
  }

  AssertEq(fields.string_, 'Panda string!!');
  AssertEq(fields.nullableString_, 'nullable string');
  fields.nullableString_ = null;
  AssertEq(fields.nullableString_, null);
}

function testMethods() {
  AssertEq(Methods.StaticSumDouble(THREE, TWENTY_TWO_HUNDREDTH).toFixed(PRECISION), (THREE + TWENTY_TWO_HUNDREDTH).toFixed(PRECISION));

  const o = new Methods();
  AssertEq(o.IsTrue(false), false);

  AssertEq(o.SumString('Hello from ', 'Panda!!!!'), 'Hello from Panda!!!!');
  AssertEq(o.SumIntArray([ONE, TWO, THREE, FOUR, FIVE]), FIFTEEN);
  AssertEq(o.OptionalString('Optional string'), 'Optional string');

  // NOTE(ivagin): enable when supported by runtime #12808
  // NOTE(vpukhov): optional methods produce overload sets
  if (false) {
    // AssertEq(o.OptionalString(undefined), undefined);
    // AssertEq(o.OptionalString(), undefined);
  }

  // NOTE(ivagin): enable when supported by interop #12808
  if (false) {
    AssertEq(o.SumIntVariadic(ONE, TWO, THREE, FOUR, FIVE).toFixed(PRECISION), (FIFTEEN).toFixed(PRECISION));
  }
}

export class DerivedGeneric<T, U, V> extends BaseGeneric<T, U> implements IGeneric0<U>, IGeneric1<IGeneric0<V>> {
  I0Method(a: U): U { return a; };
  I1Method(a: IGeneric0<V>): IGeneric0<V> { return a; };
}

function testClasses() {
  AssertEq(ETSEnum.Val0, ZERO);
  AssertEq(ETSEnum.Val1, ONE);
  AssertEq(ETSEnum.Val2, TWO);

  const o = new External();
  AssertEq(o.o1.val, 322);
  AssertEq(o.o2.val, 323);
  AssertEq(o.o3.val, 324);

  const d = new Derived(TWO, THREE);
  AssertEq(d.a, TWO);
  AssertEq(d.b, THREE);
}

function testGenerics() {
  const tStr = new GenericClass<String>();
  AssertEq(tStr.identity('Test generic class'), 'Test generic class');
  const tNumber = new GenericClass<Number>();
  AssertEq(tNumber.identity(FIVE).toFixed(PRECISION), (FIVE).toFixed(PRECISION));
}

function testDynObj() {
  const dynObjWrapper = new DynObjWrapper();
  AssertEq(dynObjWrapper.dynObj_.val, TEN);
  AssertEq(dynObjWrapper.dynObj_.GetDynVal(), TWENTY);
}

function testPromise() {
  // NOTE(ivagin): enable when supported by interop #12808
  if (false) {
    const promiseWrapper = new PromiseWrapper();
    promiseWrapper.promise_.then((v: string) => AssertEq(v, 'Panda string'));
  }
  AsyncIdentity(TEN).then((v) => AssertEq(v, TEN));
}

runTest();
