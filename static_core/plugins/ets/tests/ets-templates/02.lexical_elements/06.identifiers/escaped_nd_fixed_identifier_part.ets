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
/*---
desc: Chapter 2 Identifiers - a fixed Unicode escape for an Nd character is accepted as IdentifierPart.
---*/

function main(): void {
    let value\u0030: int = 30
    arktest.assertEQ(value\u0030, 30, 'fixed escaped Nd IdentifierPart must resolve to the declared name')
}
