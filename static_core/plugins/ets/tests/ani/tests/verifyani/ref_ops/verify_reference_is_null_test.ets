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

native function foo(str: null | String): void

function checkStackRef(): void {
    foo(null)
}

native function anotherFrame(str: String): long
native function checkRefFromAnotherFrame(): long
native function funcAddress(str: String): long
native function badStackRef(): long

function getVar(): String {
    return "var"
}
