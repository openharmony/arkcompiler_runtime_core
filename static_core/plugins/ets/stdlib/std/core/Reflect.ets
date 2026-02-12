/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
 * @file Defines the Reflect for ArkTS
 * @kit ArkTS
 */


package std.core

native function isLiteralInitializedInterfaceImpl(target: Object): boolean
native function isFuncObjAsyncImpl(target: Object): boolean

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
* Checks if the given object is an interface initialized with a literal.
*
* @param { Object } target The object to check.
* @returns { boolean } Returns true if `target` is an interface initialized with a literal; otherwise returns false.
* @syscap SystemCapability.Utils.Lang
* @FaAndStageModel
* @since 24
*/
export function isLiteralInitializedInterface(target: Object): boolean {
    return isLiteralInitializedInterfaceImpl(target)
}

/**
*  Determines if a functional object was lowered
 * from an async function
 * @namespace reflect
 * @param { Function } target The functional object to check
 * @returns a boolean indicating whether or not the target class annotated
 *          with `std.core.AsyncFunctionObject` annotation
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isFuncObjAsync(target: Function): boolean {
    return isFuncObjAsyncImpl(target)
}

/*
 * Returns public instance getters of a class and its parents
 *
 * @param { Class } targetClass Class to get fields from
 * @returns { Array<InstanceMethod> } A new array instance filled with all the class getters
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function getInstanceGettersRecursive(targetClass: Class): Array<InstanceMethod> {
    let res = new Array<InstanceMethod>()
    let cls: Class | undefined = targetClass
    while (cls != undefined) {
        for (let method of cls!.getInstanceMethods()) {
            if (method.isGetter()) {
                res.push(method)
            }
        }
        cls = cls!.getSuper()
    }
    return res
}

/**
 * Returns public instance fields of a class and its parents
 *
 * @param { Class } targetClass Class to get fields from
 * @returns { Array<InstanceField> } A new array instance filled with all the class fields
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function getInstanceFieldsRecursive(targetClass: Class): Array<InstanceField> {
    let res = new Array<InstanceField>()
    let cls: Class | undefined = targetClass
    while (cls != undefined) {
        for (let field of cls!.getInstanceFields()) {
            res.push(field)
        }
        cls = cls!.getSuper()
    }
    return res
}

} // namespace reflect
