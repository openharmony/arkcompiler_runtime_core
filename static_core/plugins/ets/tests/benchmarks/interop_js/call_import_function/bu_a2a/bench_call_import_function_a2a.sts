/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

// NOTE: imported functions from STS to STS does not work

function stsVoid(a: number, b: number): void {}
function returnAnonymous() {
    return (a: number, b: number): void => {}
}

/**
 * @State
 * @Tags interop, bu_a2a
 */
class CallImportFunctionA2a {

    /**
     * @Setup
     */
    public setup(): void {
        console.log('Starting');
    }

    /**
     * @Benchmark
     */
    test(): number {
        const anonymous = returnAnonymous();

        //NOTE: issue(19307) uncomment this after fix call external fu or init class inside function 
        // this.callFunction(anonymous, 1000000, 1000000, 'anonymous function');

        // this.callFunction(jsVoid, 1000000, 1000000, 'function');

        return 1;
    }

    callFunction(fun: (argOne: number, argTwo: number) => void,
        argOne: number, 
        argTwo: number,
        target: string,
        ): void {
        fun(argOne, argTwo)
    }
}
