/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

let etsVm = globalThis.gtest.etsVm;
let testAll = etsVm.getFunction('Ltest_static_overload/ETSGLOBAL;', 'testAll');

export class OverloadClass {
    overloadFn(arg: number): string {
        return 'number';
    }
    overloadFn(arg: string): string {
        return 'string';
    }
    overloadFn(arg: boolean): string {
        return 'boolean';
    }
    overloadFn(arg1: number, arg2: string): string {
        return 'number-string';
    }
    overloadFn(arg1: string, arg2: number): string {
        return 'string-number';
    }
    overloadFn(arg1: number, arg2: number, arg3: number): string {
        return 'number-number-number';
    }
    overloadFn(arg1: string, arg2: string, arg3: string): string {
        return 'string-string-string';
    }

    overloadFn(arg1?: any, arg2?: any, arg3?: any): string {
        if (arguments.length === 1) {
            if (typeof arg1 === 'number') { return 'number'; }
            if (typeof arg1 === 'string') { return 'string'; }
            if (typeof arg1 === 'boolean') { return 'boolean'; }
        } else if (arguments.length === 2) {
            if (typeof arg1 === 'number' && typeof arg2 === 'string') { return 'number-string'; }
            if (typeof arg1 === 'string' && typeof arg2 === 'number') { return 'string-number'; }
        } else if (arguments.length === 3) {
            if (typeof arg1 === 'number' && typeof arg2 === 'number' && typeof arg3 === 'number') { return 'number-number-number'; }
            if (typeof arg1 === 'string' && typeof arg2 === 'string' && typeof arg3 === 'string') { return 'string-string-string'; }
        }
        return 'unknown';
    }

    static overloadStaticFn(arg: number): string {
        return 'number';
    }
    static overloadStaticFn(arg: string): string {
        return 'string';
    }
    static overloadStaticFn(arg: boolean): string {
        return 'boolean';
    }
    static overloadStaticFn(arg1: number, arg2: string): string {
        return 'number-string';
    }
    static overloadStaticFn(arg1: string, arg2: number): string {
        return 'string-number';
    }
    static overloadStaticFn(arg1: number, arg2: number, arg3: number): string {
        return 'number-number-number';
    }
    static overloadStaticFn(arg1: string, arg2: string, arg3: string): string {
        return 'string-string-string';
    }

    static overloadStaticFn(arg1?: any, arg2?: any, arg3?: any): string {
        if (arguments.length === 1) {
            if (typeof arg1 === 'number') { return 'number'; }
            if (typeof arg1 === 'string') { return 'string'; }
            if (typeof arg1 === 'boolean') { return 'boolean'; }
        } else if (arguments.length === 2) {
            if (typeof arg1 === 'number' && typeof arg2 === 'string') { return 'number-string'; }
            if (typeof arg1 === 'string' && typeof arg2 === 'number') { return 'string-number'; }
        } else if (arguments.length === 3) {
            if (typeof arg1 === 'number' && typeof arg2 === 'number' && typeof arg3 === 'number') { return 'number-number-number'; }
            if (typeof arg1 === 'string' && typeof arg2 === 'string' && typeof arg3 === 'string') { return 'string-string-string'; }
        }
        return 'unknown';
    }
}

export let instance = new OverloadClass();

function main(): void {
    testAll();
}

main();
