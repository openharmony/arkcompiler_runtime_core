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

import {
  funcReturnTypeNull,
  funcReturnTypeUndefined,
  funcArgType
} from "js_code"

function TestFuncCallArg(): boolean{
  if (funcReturnTypeNull() as null !== null) {
      return false;
  }

  if (funcReturnTypeUndefined() !== undefined) {
      return false;
  }

  let arg13: undefined = undefined;
  if (funcArgType(arg13) !== undefined) {
      return false;
  }

  let arg14: null = null;
  if (funcArgType(arg14) as null !== null) {
      return false;
  }
  return true;
}