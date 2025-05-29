/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

import {
	evaluateNumber,
	ExampleClass,
	ClassWithEmptyConstructor,
	evaluateObject,
	MyNamespace,
	evaluateArray,
} from 'module';

//! CHECKER      jscallEmpty JIT
//! RUN          force_jit: true, entry: "jscallEmpty", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jscallEmpty"
//! METHOD       "jscallEmpty"
//! PASS_AFTER   "IrBuilder"


function jscallEmpty(): int {
	evaluateNumber();
	return 0;
}

//! CHECKER      jsnewEmpty JIT
//! RUN          force_jit: true, entry: "jsnewEmpty", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jsnewEmpty"
//! METHOD       "jsnewEmpty"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NOT     "Intrinsic.JSRuntimeGetPropertyInt"
//! INST_NOT     /JSRuntime.*Value/ # no casts from JSValue to primitive
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyDouble", 2
//! INST_COUNT   /i32.*Cast f64/, 2

function jsnewEmpty(): int {
	let resObj = new ClassWithEmptyConstructor();
	let resGetV0 = resObj.v0 as int;
	let resGetV1 = resObj.v1 as int;
	if (resGetV0 == 42 && resGetV1 == 42) {
		return 0;
	} else {
		return 1;
	}
}

//! CHECKER      jscallStaticMethodEmpty JIT
//! RUN          force_jit: true, entry: "jscallStaticMethodEmpty", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jscallStaticMethodEmpty"
//! METHOD       "jscallStaticMethodEmpty"
//! PASS_AFTER   "IrBuilder"


function jscallStaticMethodEmpty(): int {
	ExampleClass.emptyMethod();
	return 0;
}

//! CHECKER      jscallObject JIT
//! RUN          force_jit: true, entry: "jscallObject", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jscallObject"
//! METHOD       "jscallObject"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.JSRuntimeGetPropertyDouble"
//! INST_NEXT    /i32.*Cast f64/
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyDouble"
//! INST_NEXT    /i32.*Cast f64/

function jscallObject(): int {
	let resObj = new ExampleClass(42, 42);
	let res = evaluateObject(resObj);
	let resGetV0 = res.v0 as int;
	let resGetV1 = res.v1 as int;
	if (resGetV0 == 84 && resGetV1 == 84) {
		return 0;
	} else {
		return 1;
	}
}

//! CHECKER      jsnewObject JIT
//! RUN          force_jit: true, entry: "jsnewObject", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jsnewObject"
//! METHOD       "jsnewObject"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyJSValue", 2
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyDouble", 4

function jsnewObject(): int {
	let resObj = new ExampleClass(42, 42);
	let res = new ExampleClass(resObj, resObj);
	let resGetV0 = res.v0;
	let resGetV1 = res.v1;
	if ((resGetV0.v0 as int) == 42 && (resGetV0.v1 as int) == 42 && (resGetV1.v0 as int) == 42 && (resGetV1.v1 as int) == 42) {
		return 0;
	} else {
		return 1;
	}
}

//! CHECKER      jsnewSetPropertyObject JIT
//! RUN          force_jit: true, entry: "jsnewSetPropertyObject", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jsnewSetPropertyObject"
//! METHOD       "jsnewSetPropertyObject"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.JSRuntimeSetPropertyJSValue", 2
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyJSValue", 2
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyDouble", 4

function jsnewSetPropertyObject(): int {
	let resObj = new ClassWithEmptyConstructor();
	let res = new ClassWithEmptyConstructor();
	resObj.v0 = res;
	resObj.v1 = res;
	let resGetV0 = resObj.v0;
	let resGetV1 = resObj.v1;
	if ((resGetV0.v0 as int) == 42 && (resGetV0.v1 as int) == 42 && (resGetV1.v0 as int) == 42 && (resGetV1.v1 as int) == 42) {
		return 0;
	} else {
		return 1;
	}
}

//! CHECKER      jscallMethodObject JIT
//! RUN          force_jit: true, entry: "jscallMethodObject", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jscallMethodObject"
//! METHOD       "jscallMethodObject"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyDouble", 2

function jscallMethodObject(): int {
	let resObj = new ExampleClass(42, 42);
	let res = evaluateObject(resObj);
	let resGetV0 = res.v0 as int;
	let resGetV1 = res.v1 as int;
	if (resGetV0 == 84 && resGetV1 == 84) {
		return 0;
	} else {
		return 1;
	}
}

//! CHECKER      jscallMethodSimple JIT
//! RUN          force_jit: true, entry: "jscallMethodSimple", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jscallMethodSimple"
//! METHOD       "jscallMethodSimple"
//! PASS_AFTER   "IrBuilder"

function jscallMethodSimple(): int {
	let resObj = new ExampleClass(42, 42);
	let res = resObj.instanceEvaluateNumber() as int;
	if (res == 84) {
		return 0;
	} else {
		return 1;
	}
}

//! CHECKER      jscallString JIT
//! RUN          force_jit: true, entry: "jscallString", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jscallString"
//! METHOD       "jscallString"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.JSRuntimeGetValueString"
//! INST_NEXT    "Intrinsic.StdCoreStringEquals"

function jscallString(): int {
	let v0: String = 'hello';
	let v1: String = 'world';
	let res = evaluateNumber(v0, v1) as String;
	if (res == 'helloworld') {
		return 0;
	} else {
		return 1;
	}
}

//! CHECKER      jsnewString JIT
//! RUN          force_jit: true, entry: "jsnewString", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jsnewString"
//! METHOD       "jsnewString"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyString", 2
//! INST_NOT     "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NOT     "Intrinsic.JSRuntimeGetValueString"

function jsnewString(): int {
	let v0: String = 'hello';
	let v1: String = 'world';
	let resObj = new ExampleClass(v0, v1);
	let resGetV0 = resObj.v0 as String;
	let resGetV1 = resObj.v1 as String;
	if (resGetV0 == v0 && resGetV1 == v1) {
		return 0;
	} else {
		return 1;
	}
}

//! DISABLED_CHECKER      jsnewSetPropertyString JIT
//! RUN          force_jit: true, entry: "jsnewSetPropertyString", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jsnewSetPropertyString"
//! METHOD       "jsnewSetPropertyString"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "Intrinsic.JSRuntimeSetPropertyJSValue"
//! INST_COUNT   "Intrinsic.JSRuntimeNewJSValueString", 2
//! INST_COUNT   "Intrinsic.JSRuntimeSetPropertyString", 2
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyString", 2
//! INST_NOT     "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NOT     "Intrinsic.JSRuntimeGetValueString"
//! PASS_AFTER   "Cleanup"
//! INST_NOT     "Intrinsic.JSRuntimeNewJSValueString"

function jsnewSetPropertyString(): int {
	let v0: String = 'hello';
	let v1: String = 'world';
	let resObj = new ClassWithEmptyConstructor();
	resObj.v0 = v0;
	resObj.v1 = v1;
	let resGetV0 = resObj.v0 as String;
	let resGetV1 = resObj.v1 as String;
	if (resGetV0 == v0 && resGetV1 == v1) {
		return 0;
	} else {
		return 1;
	}
}

//! CHECKER      jscallStaticMethodString JIT
//! RUN          force_jit: true, entry: "jscallStaticMethodString", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jscallStaticMethodString"
//! METHOD       "jscallStaticMethodString"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "Intrinsic.JSRuntimeNewJSValueString"
//! INST    "Intrinsic.JSRuntimeGetValueString"

function jscallStaticMethodString(): int {
	let v0: String = 'hello';
	let v1: String = 'world';
	let res = ExampleClass.evaluateNumber(v0, v1) as String;
	if (res == 'helloworld') {
		return 0;
	} else {
		return 1;
	}
}

function jscallArray(): int {
	let v0: int[] = new int[10];
	for (let i = 0; i < 10; ++i) {
		v0[i] = i * i;
	}

	let res = evaluateArray(v0, 10);
	for (let i = 0; i < 10; ++i) {
		if ((res[i] as int) != 2 * i * i) {
			return 1;
		}
	}

	return 0;
}

function jsnewArray(): int {
	let v0: int[] = new int[10];
	for (let i = 0; i < 10; ++i) {
		v0[i] = i * i;
	}

	let resObj = new ExampleClass(v0, v0);
	let resGetV0 = resObj.v0;
	let resGetV1 = resObj.v1;

	for (let i = 0; i < 10; ++i) {
		if ((resGetV0[i] as int) != v0[i] || (resGetV1[i] as int) != v0[i]) {
			return 1;
		}
	}
	return 0;
}

function jsnewSetPropertyArray(): int {
	let v0: int[] = new int[10];
	for (let i = 0; i < 10; ++i) {
		v0[i] = i * i;
	}

	let resObj = new ClassWithEmptyConstructor();

	resObj.v0 = v0;
	resObj.v1 = v0;
	let resGetV0 = resObj.v0;
	let resGetV1 = resObj.v1;

	for (let i = 0; i < 10; ++i) {
		if ((resGetV0[i] as int) != v0[i] || (resGetV1[i] as int) != v0[i]) {
			return 1;
		}
	}
	return 0;
}

function jscallStaticMethodArray(): int {
	let v0: int[] = new int[10];
	for (let i = 0; i < 10; ++i) {
		v0[i] = i * i;
	}

	let res = ExampleClass.evaluateArray(v0, 10);
	for (let i = 0; i < 10; ++i) {
		if ((res[i] as int) != 2 * i * i) {
			return 1;
		}
	}
	return 0;
}

function testNamespace(): int {
	let v0: int = 1;
	let v1: String = 'Pinky';
	let resObj = MyNamespace.createKitten(v0, v1);
	let resId = resObj.id as int;
	let resName = resObj.name as String;
	if (resId == v0 && resName == v1) {
		return 0;
	} else {
		return 1;
	}
}
