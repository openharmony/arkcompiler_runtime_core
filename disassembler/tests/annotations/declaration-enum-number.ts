/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

const enum E {
    A = 42,
    B = 314
} 

@interface __$$ETS_ANNOTATION$$__Anno1 {
    // No initializer with underlying type number
    a: E = new Number(0) as number;
}

@interface __$$ETS_ANNOTATION$$__Anno2 {
    a: E = 42;
}