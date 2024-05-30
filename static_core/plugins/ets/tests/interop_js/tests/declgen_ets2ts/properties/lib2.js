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
'use strict';

class DynProperties {
  SetPropertyInternal(prop, value) {
    this[prop] = value;
  }

  GetPropertyInternal(prop) {
    return this[prop];
  }
}

function SetPropertyExternal(obj, key, value) {
  obj[key] = value;
}

function GetPropertyExternal(obj, key) {
  return obj[key];
}

exports.DynProperties = DynProperties;
exports.SetPropertyExternal = SetPropertyExternal;
exports.GetPropertyExternal = GetPropertyExternal;
