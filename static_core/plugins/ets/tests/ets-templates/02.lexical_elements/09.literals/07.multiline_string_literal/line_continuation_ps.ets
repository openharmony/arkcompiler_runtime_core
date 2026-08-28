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
desc: Chapter 2 Multiline String Literal - backslash plus U+2029 is accepted as a line continuation.
---*/

function main(): void {
    let value: string = `left\ right`
    arktest.assertTrue(value.charAt(0) == 108, 'U+2029 continuation must preserve the stable prefix')
    arktest.assertTrue(value.charAt(value.length - 1) == 116, 'U+2029 continuation must preserve the stable suffix')
}
