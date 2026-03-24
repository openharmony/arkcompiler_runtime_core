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

package arkruntime;

/**
 * Class implements ets representation of context of async function/method
 * This class and its methods must not be used by developer directly 
 */
@annotation.BytecodeAPI
export final class AsyncContext {
    private awaitee: Promise<Any> | undefined = undefined;
    private returnValue: Promise<Any> | undefined = undefined;

    private refValues: FixedArray<Any> | undefined = undefined;
    private primValues: ValueArray<long> | undefined = undefined;
    private frameOffsets: ValueArray<short> | undefined = undefined;

    private refCount: long;
    private primCount: long;

    private awaitId: long = -1;
    private compiledCode: long = 0;

    /**
     * Method creates AsyncContext represents context of current async function
     * @returns { AsyncContext } created async context
     */
    static native init(): AsyncContext;

    /**
     * Method returns current AsyncContext of async function
     * @returns { AsyncContext } current async context
     */
    static native current(): AsyncContext | undefined;

    /**
     * Method sets value to resolve associated promise  
     * @returns { Promise<Any> } resolved promise 
     */
    native resolve<T>(val: T): Promise<Awaited<T>>;

    /**
     * Method sets error to reject associated promise  
     * @returns { Promise<Any> } rejected promise 
     */
    native reject(err: Error): Promise<Any>;

    /**
     * Method returns value that was used to resolve or reject associated promise  
     * @returns { Any } async context's result 
     */
    native result(): Any;

    /**
     * Method returns value that was awaited
     * @returns { Promise<Any> } awaited promise's result 
     */
    native awaitResult(): Any;
}
