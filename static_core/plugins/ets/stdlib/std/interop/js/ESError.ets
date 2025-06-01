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

package std.interop.js;

export final class ESError extends Error {
    private err_: ESObject;
    private jserr_: JSValue;

    private constructor(err: JSValue) {
        super("ESError", "", new ErrorOptions(err as Object));
        this.err_ = new ESObject(err);
        this.jserr_ = err;
    }

    /**
     * Retrieves the name of the error from the ESObject.
     * If the property does not exist, returns "UnknownError".
     */
    override get name(): string {
        return this.err_.hasProperty("name") 
            ? this.err_.getProperty("name").toString() 
            : "UnknownError";
    }

    /**
     * Sets the name of the error in the ESObject.
     */
    override set name(val: string) {
        this.err_.setProperty("name", ESObject.wrapString(val));
    }

    /**
     * Retrieves the message of the error from the ESObject.
     * If the property does not exist, returns an empty string.
     */
    override get message(): string {
        return this.err_.hasProperty("message") 
            ? this.err_.getProperty("message").toString() 
            : "";
    }

    /**
     * Sets the message of the error in the ESObject.
     */
    override set message(val: string) {
        this.err_.setProperty("message", ESObject.wrapString(val));
    }

    /**
     * Retrieves the cause of the error from the ESObject.
     * If the property does not exist, returns undefined.
     */
    override get cause(): ESObject | undefined {
        return this.err_.hasProperty("cause") 
            ? this.err_.getProperty("cause") 
            : undefined;
    }

    /**
     * Sets the cause of the error in the ESObject.
     */
    override set cause(val: Object | undefined) {
        if (val !== undefined) {
            this.err_.setProperty("cause", val);
        }
    }

    /**
     * Retrieves the ESObject associated with this error.
     */
    getValue(): ESObject {
        return this.err_;
    }

    getJsError(): JSValue {
        return this.jserr_;
    }
}
