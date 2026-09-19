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
 * The class under test, version 2: the patch. Only the `tag()` body differs from
 * `target/t.ets` --- no structural change, so the reload is accepted --- and the new
 * literal is what proves that a handle cached before the reload dispatches into the
 * new generation afterwards.
 */

export class Greeter {
    public n: int = 0;

    public tag(): string {
        return 'V2';
    }
}
