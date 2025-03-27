/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

package api;

const TypeErrorCodeId: number = 401;
export const OutOfBoundsErrorCodeId: number = 10200001;
export const SyntaxErrorCodeId: number = 10200002;
export const IncorrectBufferSizeId: number = 10200009;

/**
 * Represents an error that occurs when using a function under the API package in an incorrect manner
 */
export class BusinessError extends Error {
    code: number;
    constructor(msg: string) {
        super(msg);
        this.name = 'BusinessError';
        this.code = TypeErrorCodeId;
    }

    constructor(msg: string, code: number) {
        super(msg);
        this.name = 'BusinessError';
        this.code = code;
    }
}
