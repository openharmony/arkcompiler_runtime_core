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

export let nop: () => void = function (): void {};

export let identity: <T>(v: T) => T = function <T>(v: T): T {
	return v;
};

export let stringifyValue: (v: any) => string = function (v: any): string {
	return typeof v + ':' + v;
};

export let stringifyArgs: (...args: any[]) => string = function (...args: any[]): string {
	return args.toString();
};

export let setProtoConstructor: (v: any) => Function = function (v: any): Function {
	return Object.getPrototypeOf(v).constructor;
};

export let applyArgs: <T>(fn: (...args: any[]) => T, ...args: any[]) => T = function <T>(
    fn: (...args: any[]) => T,
    ...args: any[]
): T {
    return fn(...args);
};

export let throwValue: (v: any) => never = function (v: any): never {
	throw v;
};

export let log: (...args: any[]) => void = function (...args: any[]): void {
	console.log(`${args.join(' ')}`);
};

export let getProp: <T, K extends keyof T>(v: T, p: K) => T[K] = function <T, K extends keyof T>(
	v: T,
	p: K
): T[K] {
	return v[p];
};

export let sum: (a: number, b: number) => number = function (a: number, b: number): number {
	return a + b;
};

export let makeCar: (v: string) => void = function (v) {
	this.color = v;
};

export let makeTestProxy: (this: { foo?: any }) => void = function (
    this: { foo?: any }
): void {
	Object.defineProperty(this, 'foo', {
		get: function (): never {
			throw Error('get exception');
		},
		set: function (): never {
			throw Error('set exception');
		},
	});
};
