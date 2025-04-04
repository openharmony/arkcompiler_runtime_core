/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
flags: [module]
---*/


import { a } from './module-multi-export.js';
import { b1, b2 } from './module-multi-export.js';
import { c } from './module-multi-export.js';

import dd from './module-multi-export.js';

import ee, * as n from './module-multi-export.js';
import * as ns from './module-multi-export.js';

import { f as ff } from './module-multi-export.js';
import { g, h as hh } from './module-multi-export.js';

import './module-multi-export.js';

class ClassA {
  fun(a, b) { return a + b; }
  str = 'test';
}