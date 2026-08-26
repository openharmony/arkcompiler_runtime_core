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

/*
 * The class under test, version 1: the one loaded when the native driver caches its
 * `ani_method` / `ani_field` handles.
 *
 * The file name MUST be `t.ets` in both this directory and `patch/`: es2panda derives
 * the module name from the source basename, so target and patch produce identical
 * record descriptors (`t.Greeter`), which is what makes the patch replace the class
 * the loaded handles belong to.
 */

export class Greeter {
    public n: int = 0;

    public tag(): string {
        return 'V1';
    }
}
