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

// Entry module for OHMurl import tests:
// greet is imported via a side-effect OHMurl request (creates an on-open
// external-module placeholder with the same name as a later carrier),
// versioned is not requested here (used for fresh with-version imports).
import '@normalized:N&&&modules/greet&';

print('entry evaluated');
