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
 * WITHOUT WARRANTIES OR CONDITIONS of ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package std.core;
// Note: This file initialize all the `const` property that non-immediately initialized(spec-issue 217).
static {
    // 5 lower bits stores kind's id
    TypeKindMask = ((1 << 6) - 1).toByte();
    
    ObjectType = Type.of(new Object()) as ClassType;
    console = new Console();
    coroutine = new Coroutine();
}
