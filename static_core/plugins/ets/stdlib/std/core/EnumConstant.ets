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
export final class EnumConstant {
    private name: string = ""
    private typ: Type | undefined
    private intVal: int
    private strVal: string = ""

    private constructor () {}

    public getType(): Type {
        return this.typ!
    }

    public getName(): string {
        return this.name
    }

    public getValue(): int {
        return this.intVal
    }

    public getStringValue(): string {
        return this.strVal
    }

    public hasValue(): boolean {
        // TODO(shumilov-petr): not implemented
        return false
    }

    public hasStringValue(): boolean {
        // TODO(shumilov-petr): not implemented
        return false
    }

    public static create(name: string, typ: Type, v: int): EnumConstant {
        // TODO(shumilov-petr): not implemented
        throw new Error("not implemented")
    }

    public static create(name: string, typ: Type, v: string): EnumConstant {
        // TODO(shumilov-petr): not implemented
        throw new Error("not implemented")
    }

    public override toString(): string {
        // TODO(shumilov-petr): not implemented
        throw new Error("not implemented")
    }
}
