/**
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

package error.test;

export let err = new Error('This is a Error');

export function ErrorFunc(message: string): void {
  throw new Error(message);
}

export class CustomError extends Error {
  code: number;
  constructor(message: string, code: number) {
    super(message);
    this.name = "CustomError";
    this.code = code;
  }
}

export function CustomErrorFunc(message: string, code: number): void {
  throw new CustomError(message, code);
}