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
exports.callableTestClassInstance =
	exports.CallableTestClass =
	exports.callBoundFunction =
	exports.callConstructedFunction =
	exports.callArrowFunction =
	exports.callAnonymousFunction =
	exports.callNamedFunction =
		void 0;
function callNamedFunction(x, y) {
	return x + y;
}
exports.callNamedFunction = callNamedFunction;
let callAnonymousFunction = function (x, y) {
	return x + y;
};
exports.callAnonymousFunction = callAnonymousFunction;
let callArrowFunction = (x, y) => x + y;
exports.callArrowFunction = callArrowFunction;
// CC-OFFNXT(G.MET.08) test specific
exports.callConstructedFunction = new Function('a', 'b', 'return a + b');
exports.callBoundFunction = callNamedFunction.bind(null);
class CallableTestClass {
	constructor() {
		this.callNamedFunction = callNamedFunction.bind(this);
		this.callAnonymousFunction = exports.callAnonymousFunction.bind(this);
		this.callArrowFunction = exports.callArrowFunction.bind(this);
		this.callConstructedFunction = exports.callConstructedFunction.bind(this);
		this.callBoundFunction = exports.callBoundFunction.bind(this);
	}
}
exports.CallableTestClass = CallableTestClass;
exports.callableTestClassInstance = new CallableTestClass();
const test = () => {
	console.log(
		[callNamedFunction, exports.callAnonymousFunction, exports.callArrowFunction, exports.callConstructedFunction, exports.callBoundFunction].map((x) =>
			x(10, 5)
		)
	);
};
test();
