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
    evaluateArray
} from "module"

function jscallEmpty(): int {
	evaluateNumber();
	return 0;
}

function jsnewEmpty(): int {
	let resObj = new ClassWithEmptyConstructor();
	let resGetV0 = resObj.v0 as number;
	let resGetV1 = resObj.v1 as number;
	if (resGetV0 == 42 && resGetV1 == 42) {
		return 0;
	} else {
		return 1;
	}
}

function jscallStaticMethodEmpty(): int {
	ExampleClass.emptyMethod();
	return 0;
}

function jscallObject(): int {
	let resObj = new ExampleClass(42, 42);
	let res = evaluateObject(resObj);
	let resGetV0 = res.v0 as number;
	let resGetV1 = res.v1 as number;
	if (resGetV0 == 84 && resGetV1 == 84) {
		return 0;
	} else {
		return 1;
	}
}

function jsnewObject(): int {
	let resObj = new ExampleClass(42, 42);
	let res = new ExampleClass(resObj, resObj);
	let resGetV0 = res.v0 as ExampleClass;
	let resGetV1 = res.v1 as ExampleClass;
	if ((resGetV0.v0 as number) == 42 && (resGetV0.v1 as number) == 42 && (resGetV1.v0 as number) == 42 && (resGetV1.v1 as number) == 42) {
		return 0;
	} else {
		return 1;
	}
}

function jsnewSetPropertyObject(): int {
	let resObj = new ClassWithEmptyConstructor();
	let res = new ClassWithEmptyConstructor();
	resObj.v0 = res;
	resObj.v1 = res;
	let resGetV0 = resObj.v0 as ClassWithEmptyConstructor;
	let resGetV1 = resObj.v1 as ClassWithEmptyConstructor;
	if ((resGetV0.v0 as number) == 42 && (resGetV0.v1 as number) == 42 && (resGetV1.v0 as number) == 42 && (resGetV1.v1 as number) == 42) {
		return 0;
	} else {
		return 1;
	}
}

function jscallMethodObject(): int {
	let resObj = new ExampleClass(42, 42);
	let res = evaluateObject(resObj);
	let resGetV0 = res.v0 as number;
	let resGetV1 = res.v1 as number;
	if (resGetV0 == 84 && resGetV1 == 84) {
		return 0;
	} else {
		return 1;
	}
}

function jscallMethodSimple(): int {
	let resObj = new ExampleClass(42, 42);
	let res = resObj.instanceEvaluateNumber() as number;
	if (res == 84) {
		return 0;
	} else {
		return 1;
	}
}

function jscallString(): int {
	let v0: String = 'hello';
	let v1: String = 'world';
	let res = evaluateNumber(v0, v1);
	if (res == 'helloworld') {
		return 0;
	} else {
		return 1;
	}
}

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

function jscallStaticMethodString(): int {
	let v0: String = 'hello';
	let v1: String = 'world';
	let res = ExampleClass.evaluateNumber(v0, v1);
	if (res == 'helloworld') {
		return 0;
	} else {
		return 1;
	}
}

function jscallArray(): int {
	let v0: FixedArray<int> = new int[10];
	for (let i = 0; i < 10; ++i) {
		v0[i] = i * i;
	}

	let res = evaluateArray(v0, 10);
	for (let i = 0; i < 10; ++i) {
		if (res[i] != 2 * i * i) {
			return 1;
		}
	}

	return 0;
}

function jsnewArray(): int {
	let v0: FixedArray<int> = new int[10];
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
	let v0: FixedArray<int> = new int[10];
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
	let v0: FixedArray<int> = new int[10];
	for (let i = 0; i < 10; ++i) {
		v0[i] = i * i;
	}

	let res = ExampleClass.evaluateArray(v0, 10);
	for (let i = 0; i < 10; ++i) {
		if (res[i] != 2 * i * i) {
			return 1;
		}
	}
	return 0;
}

function testNamespace(): int {
	let v0: int = 1;
	let v1: String = 'Pinky';
	let resObj = MyNamespace.createKitten(v0, v1);
	let resId = resObj.id;
	let resName = resObj.name;
	if (resId == v0 && resName == v1) {
		return 0;
	} else {
		return 1;
	}
}
