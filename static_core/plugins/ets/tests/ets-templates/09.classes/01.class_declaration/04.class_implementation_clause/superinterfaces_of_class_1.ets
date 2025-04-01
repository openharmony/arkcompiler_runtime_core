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
desc: Implement interface with superinterface.
assert: C has some direct superinterface J for which I is a superinterface, using the definition of "superinterface of an interface" given in link to interface’s superinterfaces.
---*/

interface I { }

interface J extends I { }

final class C implements J { }

function main(): void {
  let c: I = new C();
}
