/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

package evalPatchFunction;

import { S } from "../local_variable_user_class.base";

class evalPatchFunction {
    private static evalPatchFunction() {
        let a: S = DebuggerAPI.getLocalObject(0) as S;

        const localVar = new S();
        // Result won't be returned due to non-primitive type.
        a.s.str = a.s.str + localVar.s.str + '2';

        DebuggerAPI.setLocalObject(0, a);
    }
}
