/*
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

const MAX_ARGUMENTS = 2;
class A {
	bar(func, ...args) {
		if (args.length > MAX_ARGUMENTS) {
			throw new Error('Too many method arguments');
		}
		return func(...args.slice(0, MAX_ARGUMENTS));
	}
}

exports.A = A;
