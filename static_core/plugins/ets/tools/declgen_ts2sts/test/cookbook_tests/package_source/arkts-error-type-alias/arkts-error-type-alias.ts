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
export class APIError extends Error{
  message: string = 'APIError'
}
export class APIError1 implements Error{
  name: string ="APIError1"
  stack?: string | undefined
  cause?: unknown
  message: string = 'APIError1'
}
export class APIError2 extends APIError1{}
export class APIError3 extends APIError2{}
export class APIError4 extends APIError{
  message: string = 'APIError4'
}