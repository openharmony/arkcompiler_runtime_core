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

/**
 * Represents an error that occurs when using a function under the API package in an incorrect manner
 */
export class BusinessError<T = void> extends Error {
    code: number;
    data?: T;

    constructor() {
        super();
        this.code = 0;
    }

    constructor(code: number, error: Error) {
        super(error.name, error.message, new ErrorOptions(error.cause));
        this.code = code;
    }

    constructor(code: number, data: T, error: Error) {
        super(error.name, error.message, new ErrorOptions(error.cause));
        this.code = code;
        this.data = data;
    }
}