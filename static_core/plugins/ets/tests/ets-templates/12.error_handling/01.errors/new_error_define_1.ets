/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

/*---
desc: Define new error from derived class of Error.
assert: Error is the base class of all the errors. Usually there is no need to define a new error class, as error classes for various cases are defined in a standard library, like DivideByZeroError. However, a developer may define a new error using Error or any derived class as the base of new class.
tags: [compile-only]
---*/

class SuperError extends Error {}

class TestError extends SuperError {}
