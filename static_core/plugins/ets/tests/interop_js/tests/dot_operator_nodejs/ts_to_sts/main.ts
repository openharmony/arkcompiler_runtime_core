/**
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

export const tsInt = 1;
export const tsString = 'string';
export const Obj = {
    name: 'test',
    inside: {
        name: 'test',
    },
    sum: (): number => tsInt,
    arr: [1, 2, 3],
};

type OptionalProp = {
    name?: string,
    secondName: null | undefined
};

export const notNullishObj: OptionalProp = {
    secondName: null,
};
