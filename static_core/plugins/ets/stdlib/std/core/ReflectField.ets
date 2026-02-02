/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
/**
 * @file Defines the ReflectField for ArkTS
 * @kit ArkTS
 */


package std.core;

/**
 * Core namespace for reflection operations, providing access to metadata such as classes, interfaces,
 * methods, and properties at runtime.
 *
 * @namespace reflect
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export namespace reflect {

/**
 * Represents field of class or interface
 *
 */
class Field {
    protected name: string | undefined
    protected ownerType: Class
    protected attributes: int
    protected fieldPtr: long
    protected accessMod: byte

    /**
     * Protected native constructor for the Field class.
     *
     */
    protected native constructor()

    /**
     * Checks if the field is readonly.
     *
     * @returns { boolean } Returns true if the field is readonly; otherwise returns false.
     *
     */
    public isReadonly(): boolean {
        return (this.attributes & Attributes.READONLY) != 0
    }

    /**
     * Checks if the field has public access permission.
     *
     * @returns { boolean } Returns true if the field is public; otherwise returns false.
     *
     */
    public isPublic(): boolean {
        return (this.accessMod == AccessModifier.PUBLIC)
    }

    /**
     * Checks if the field has private access permission.
     *
     * @returns { boolean } Returns true if the field is private; otherwise returns false.
     *
     */
    public isPrivate(): boolean {
        return (this.accessMod == AccessModifier.PRIVATE)
    }

    /**
     * Checks if the field has protected access permission.
     *
     * @returns { boolean } Returns true if the field is protected; otherwise returns false.
     *
     */
    public isProtected(): boolean {
        return (this.accessMod == AccessModifier.PROTECTED)
    }

    /**
     * Gets the type of this field.
     *
     * @returns { Class } Returns the Class object representing the type of this field.
     *
     */
    public getType(): Class {
        return Field.getTypeInternal(this.fieldPtr)
    }

    /**
     * Gets the class or interface that declares this field.
     *
     * @returns { Class } Returns the Class object representing the owner of this field.
     *
     */
    public getOwner(): Class {
        return this.ownerType
    }

    /**
     * Gets the name of this field.
     *
     * @returns { string } Returns the name of the field.
     *
     */
    public getName(): string {
        return this.name ?? Field.getNameInternal(this.fieldPtr)
    }

    public setValue(thisObj: Object | undefined, value: Any): void {
        this.ownerType.initialize()
        return this.setValueInternal(thisObj, value)
    }

    public getValue(thisObj: Object | undefined): Any {
        this.ownerType.initialize()
        return this.getValueInternal(thisObj)
    }

    /**
     * Native method for setting the field value.
     *
     * @param { Object | undefined } thisObj The object instance to set the field value for; should be undefined for static fields.
     * @param { Any } value The new value to set for the field.
     *
     */
    protected native setValueInternal(thisObj: Object | undefined, value: Any): void
    /**
     * Native method for getting the field value.
     *
     * @param { Object | undefined } thisObj The object instance to get the field value from; should be undefined for static fields.
     * @returns { Any } Returns the current value of the field.
     *
     */
    protected native getValueInternal(thisObj: Object | undefined): Any

    private static native getTypeInternal(fieldPtr: long): Class
    /**
     * Native static method for getting the field name based on field pointer.
     *
     * @param { long } fieldPtr The internal native pointer pointing to the field.
     * @returns { string } The name of the field.
     *
     */
    private static native getNameInternal(fieldPtr: long): string
}

} // namespace reflect
