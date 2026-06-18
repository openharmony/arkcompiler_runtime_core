/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

export declare class OverloadClass {
    overloadFn(arg: number): string;
    overloadFn(arg: string): string;
    overloadFn(arg: boolean): string;
    overloadFn(arg1: number, arg2: string): string;
    overloadFn(arg1: string, arg2: number): string;
    overloadFn(arg1: number, arg2: number, arg3: number): string;
    overloadFn(arg1: string, arg2: string, arg3: string): string;

    static overloadStaticFn(arg: number): string;
    static overloadStaticFn(arg: string): string;
    static overloadStaticFn(arg: boolean): string;
    static overloadStaticFn(arg1: number, arg2: string): string;
    static overloadStaticFn(arg1: string, arg2: number): string;
    static overloadStaticFn(arg1: number, arg2: number, arg3: number): string;
    static overloadStaticFn(arg1: string, arg2: string, arg3: string): string;
}

export declare let instance: OverloadClass;