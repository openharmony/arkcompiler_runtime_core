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

package std.core;

/**
 * Represents parameter of function type
 */
export final class Parameter extends Object {
    private paramType: Type | undefined // not undefined if already created
    private name: string = ""
    private attributes: int

    private constructor () {}

    public getType(): Type {
        return this.paramType!
    }

    public getName(): string {
        return this.name
    }

    public getAttributes(): int {
        return this.attributes
    }

    public isRest(): boolean {
        return (this.attributes & Attributes.REST) != 0
    }

    public isOptional(): boolean {
        return (this.attributes & Attributes.OPTIONAL) != 0
    }

    public override toString(): string {
        return this.getName() + ": " + this.getType().toString()
    }

    public equals(oth: NullishType): boolean {
        return oth instanceof Parameter &&
                this.paramType!.equals((oth as Parameter).paramType!) &&
                this.name == (oth as Parameter).name &&
                this.attributes == (oth as Parameter).attributes
    }
}
