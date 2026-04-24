'use static'
/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
export declare interface CallableInterface {
    method1(): void;
}
export declare interface OptionalMethodInterface {
    method2(): void;
    method3(a: string, b: number): string;
}
export declare interface MixedInterface {
    method2(): void;
}
export declare interface CallableInterface1 {
}
export declare interface OptionalMethodInterface1 {
}
export declare interface NormalInterface {
    method2(): void;
}
