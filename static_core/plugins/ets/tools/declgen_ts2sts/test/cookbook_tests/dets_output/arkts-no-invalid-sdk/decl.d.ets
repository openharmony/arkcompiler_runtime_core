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
import { Callback as A } from "./@ohos.base";
import { A as A2 } from "./bridge2";
export declare const task: Any;
export declare const task2: Any;
export declare const a: A<string>;
export declare const a2: A2<Any>;
export declare function testFoo(a: A<string>): A2<Any>;
export declare class TestClass {
    v1: Any;
    v2: A<number>;
    v3: A2<Any>;
}
