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

(globalThis as any).Panda = require(process.env.MODULE_PATH + '/ets_interop_js_napi.node');
const etsOpts = {
    'panda-files': process.env.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
    'boot-panda-files': `${process.env.ARK_ETS_STDLIB_PATH}:${process.env.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH}`,
    'gc-trigger-type': 'heap-trigger',
    'load-runtimes': 'ets',
    'compiler-enable-jit': 'false',
    'run-gc-in-place': 'true',
    "log-level": "info",
    "log-components": "ets_interop_js",
};
const createRes = (globalThis as any).Panda.createRuntime(etsOpts);
if (!createRes) {
    console.log('Cannot create ETS runtime');
    process.exit(1);
}

(globalThis as any).require = require;

import { SumDouble, SumString, Identity, ForEach, ReplaceFirst } from './lib1'
import { Fields, Methods } from "./lib1";
import { Simple, External, Derived, ETSEnum } from "./lib1";
import { GenericClass, BaseGeneric, IGeneric0, IGeneric1 } from "./lib1"
import { DynObjWrapper, PromiseWrapper } from "./lib1"

function AssertEq<T>(a: T, b: T) {
    // console.log(`AssertEq: '${a}' === '${b}'`)
    if (a !== b) {
        throw new Error(`AssertEq failed: '${a}' === '${b}'`)
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
    AssertEq(SumDouble(3, 0.22).toFixed(3), (3.22).toFixed(3));
    AssertEq(SumString("Hello from ", "Panda!!"), "Hello from Panda!!");

    // TODO(ivagin): enable when supported by runtime #12808
    if (false) {
        AssertEq(Identity(3.22).toFixed(3), (3.22).toFixed(3));
        AssertEq(Identity("Panda identity string"), "Panda identity string");
    }

    // TODO(ivagin): enable when supported by runtime #12808
    if (false) {
        const intArr = [1, 2, 3];
        ForEach(intArr, (e: number, idx: number) => {
            AssertEq(e, intArr[idx]);
        })
        const intStr = ["1", "2", "3"];
        ForEach(intStr, (e: string, idx: number) => {
            AssertEq(e, intStr[idx]);
        })
    }

    let simpleArray: Simple[] = [new Simple(0), new Simple(1), new Simple(2)];
    simpleArray.forEach((e: Simple, idx: number) => { AssertEq(e.val, idx); });
    AssertEq(ReplaceFirst(simpleArray, new Simple(3))[0].val, 3);
}

function testFields() {
    const fields = new Fields();
    AssertEq(fields.readonlyBoolean_, true);
    AssertEq(Fields.staticFloat_.toFixed(3), (1.1).toFixed(3));
    AssertEq(fields.double_.toFixed(3), (2.2).toFixed(3));
    AssertEq(fields.doubleAlias_.toFixed(3), (3.3).toFixed(3));
    AssertEq(fields.int_, 4);
    // TODO(ivagin): enable when supported by runtime #12808
    if (false) {
        AssertEq(fields.object_ instanceof Object, true);
        AssertEq(fields.rangeError_ instanceof RangeError, true);
        AssertEq(fields.uint8Array_ instanceof Int8Array, true);
        AssertEq(fields.uint16Array_ instanceof Int16Array, true);
        AssertEq(fields.uint32Array_ instanceof Int32Array, true);
    }

    AssertEq(fields.string_, "Panda string!!");
    AssertEq(fields.nullableString_, "nullable string");

    // TODO(ivagin): enable when supported by runtime #12808
    if (false) {
        fields.nullableString_ = undefined;
        AssertEq(fields.nullableString_, undefined);
    }
}

function testMethods() {
    AssertEq(Methods.StaticSumDouble(3, 0.22).toFixed(3), (3.22).toFixed(3));

    const o = new Methods();
    AssertEq(o.IsTrue(false), false);

    AssertEq(o.SumString("Hello from ", "Panda!!!!"), "Hello from Panda!!!!");
    AssertEq(o.SumIntArray([1, 2, 3, 4, 5]), 15);
    AssertEq(o.OptionalString("Optional string"), "Optional string");

    // TODO(ivagin): enable when supported by runtime #12808
    if (false) {
        AssertEq(o.OptionalString(), undefined);
    }

    // TODO(ivagin): enable when supported by es2panda #12808
    // AssertEq(o.SumIntVariadic(1, 2, 3, 4, 5).toFixed(3), (15).toFixed(3));
}

export class DerivedGeneric<T, U, V> extends BaseGeneric<T, U> implements IGeneric0<U>, IGeneric1<IGeneric0<V>> {
   I0Method(a: U): U { return a; };
   I1Method(a: IGeneric0<V>): IGeneric0<V> { return a; };
}

function testClasses() {
    AssertEq(ETSEnum.Val0, 0);
    AssertEq(ETSEnum.Val1, 1);
    AssertEq(ETSEnum.Val2, 2);

    const o = new External();
    AssertEq(o.o1.val, 322);
    AssertEq(o.o2.val, 323);
    AssertEq(o.o3.val, 324);

    const d = new Derived();
    // TODO(ivagin): enable when supported by runtime #12808
    if (false) {
        AssertEq(d.a, 1);
    }
    AssertEq(d.b, 2);
}

function testGenerics() {
    const tStr = new GenericClass<String>();
    // TODO(ivagin): enable when supported by runtime #12808
    if (false) {
        AssertEq(tStr.identity("Test generic class"), "Test generic class");
    }
    const tNumber = new GenericClass<Number>();
    // TODO(ivagin): enable when supported by runtime #12808
    if (false) {
        AssertEq(tNumber.identity(5), 5);
    }
}

function testDynObj() {
    const dynObjWrapper = new DynObjWrapper();
    // TODO(ivagin): enable when supported by runtime #12808
    if (false) {
        AssertEq(dynObjWrapper.dynObj_.val, 10);
        AssertEq(dynObjWrapper.dynObj_.GetDynVal(), 20);
    }
}

function testPromise() {
    // TODO(ivagin): enable when supported by runtime #12808
    if (false) {
        const promiseWrapper = new PromiseWrapper();
        promiseWrapper.promise_.then((v) => AssertEq(v, "Panda string"));
    }
    // TODO(ivagin): enable when supported by ets2ts declgen
    // AsyncIdentity(10).then((v) => AssertEq(v, 10));
}

runTest();
