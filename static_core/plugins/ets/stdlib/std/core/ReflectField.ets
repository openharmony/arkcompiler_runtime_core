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

package std.core;

export namespace reflect {

/**
 * Represents field of class or interface
 *
 */
class Field {
    protected name: string | undefined
    protected fieldType: Class
    protected ownerType: Class
    protected attributes: int
    protected fieldPtr: long
    protected accessMod: byte

    private native constructor()

    public isReadonly(): boolean {
        return (this.attributes & Attributes.READONLY) != 0
    }

    public isPublic(): boolean {
        return (this.accessMod == AccessModifier.PUBLIC)
    }

    public isPrivate(): boolean {
        return (this.accessMod == AccessModifier.PRIVATE)
    }

    public isProtected(): boolean {
        return (this.accessMod == AccessModifier.PROTECTED)
    }

    public getType(): Class {
        return this.fieldType
    }

    public getOwner(): Class {
        return this.ownerType
    }

    public getName(): string {
        return this.name ?? Field.getNameInternal(this.fieldPtr)
    }

    private static native getNameInternal(fieldPtr: long): string
}

} // namespace reflect
