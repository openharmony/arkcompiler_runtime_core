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

/*---
desc: Verify that a child namespace can access constants from multiple parent namespaces.
---*/

namespace NamespaceA {
    export const VALUE = "ValueA";
}

namespace NamespaceB {
    export const VALUE = "ValueB";
}

namespace SubNamespace {
    export const VALUE = `${NamespaceA.VALUE}-${NamespaceB.VALUE}`;
}

function main(): void {
    assertEQ(SubNamespace.VALUE, "ValueA-ValueB");
}