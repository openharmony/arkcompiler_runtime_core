'use static'
/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
export declare namespace ns {
    export function foo(_this: number): void;
}
export declare function greet(_this: string): number;
export declare const fun: (_this: number, a: number) => number;
export declare class A {
    value: number;
    foo(_this: A, a: number): void;
}
export declare interface Itest {
    foo(_this: number): void;
}
