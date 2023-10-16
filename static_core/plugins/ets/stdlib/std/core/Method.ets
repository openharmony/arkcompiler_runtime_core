/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

native function TypeAPIMethodInvoke(methodDesc: string, recv: NullishType, args: NullishType[]): NullishType
native function TypeAPIMethodInvokeConstructor(methodDesc: string, args: NullishType[]): Object

export final class Method extends Object {
    private td: string
    private name: string
    private attributes: int
    private accessMod: byte

    public getType(): FunctionType {
        return Type.resolve(this.td) as FunctionType
    }

    public getName(): string {
        return this.name
    }

    internal getTD(): string {
        return this.td
    }

    public invoke(recv: NullishType, args: NullishType[]): NullishType {
        const isCtor = (this.attributes & Attributes.CONSTRUCTOR) != 0
        const isStatic = (this.attributes & Attributes.STATIC) != 0
        if ((isStatic || isCtor) != (recv == null)) {
            throw new Error("reciever may be null only for static/ctor methods")
        }
        const thisType = this.getType()
        if (thisType.getParametersNum() != args.length) {
            throw new Error("arguments length mismatch " + thisType.getParametersNum() + " != " + args.length)
        }
        let convertedArgs: NullishType[] = new NullishType[args.length]
        for (let i = 0; i < convertedArgs.length; i++) {
            convertedArgs[i] = thisType.getParameter(i).getType().convertObject(args[i])
        }
        if (isCtor) {
            return TypeAPIMethodInvokeConstructor(this.td, convertedArgs)
        } else {
            if (!isStatic) {
                recv = thisType.getReceiverType().convertObject(recv)
            }
            return TypeAPIMethodInvoke(this.td, recv, convertedArgs)
        }
    }
}
