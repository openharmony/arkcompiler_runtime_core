'use static'
/*
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

import ___LibA_default from "./lib-a";
import __LibB_default from "./lib-b";
// This type name intentionally conflicts with the generated name for LibA
export type __LibA_default = string;
export declare const a1: ___LibA_default;
export declare const a2: ___LibA_default;
export declare const b: __LibB_default;
export declare function convert(input: ___LibA_default): __LibB_default;
export declare class C extends __LibB_default {
    some: string;
}
