/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
    Operations on variables of literal types are identical to the operations of their supertypes.
    The resulting operation type is the type specified for the operation in the supertype.
    In most cases, it is the supertype itself.
---*/

type VER = "top" | "middle" | "bottom"| "same";
type HOR = "left" | "center" | "right"| "same";
let s: string[] = ["left", "top", "center", "same", "bottom", "middle", "right"]

function main() {
    let i: (VER|HOR)[] = ["left", "top", "center", "same", "bottom", "middle", "right"]
    for(let j = 0; j < s.length; j++) {
        assertEQ( i[j], s[j] )
    }
}
