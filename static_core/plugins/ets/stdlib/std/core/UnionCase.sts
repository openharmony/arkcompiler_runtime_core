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
 * WITHOUT WARRANTIES OR CONDITIONS of ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package std.core;

// TODO(shumilov-petr): Add runtime mirror
export final class UnionCase {
    private name: string = ""
    private typ: Type | undefined // not undefined if already created
    private val: Object = new Object()

    private constructor () {}

    public isType(): boolean {
        return false
    }

    public getType(): Type {
        return this.typ!
    }

    public override toString(): string {
        // TODO(shumilov-petr): not implemented
        throw new Error("not implemented")
    }
}
