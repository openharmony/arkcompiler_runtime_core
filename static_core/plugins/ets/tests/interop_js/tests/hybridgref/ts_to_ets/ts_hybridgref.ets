/**
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
package ets_functions;

native function nativeGetRef(): ESValue;

export function etsCheckNativeGrefString(): boolean {
    let value = nativeGetRef();
    return value.toString() === "NativeGref String";
}

export function etsCheckNativeGrefNumber(): boolean {
    let obj = nativeGetRef();
    return obj.toNumber() === 123.456;
}

export function etsCheckNativeGrefObjectField(): boolean {
    let obj = nativeGetRef();
    let field = obj.getProperty("name");
    return field.toString() === "ArkTS";
}

export function etsCheckNativeGrefArray(): boolean {
    let arr = nativeGetRef();
    let len = arr.getProperty("length").toNumber();
    if (len !== 3) return false;
    return arr.getProperty(0).toNumber() === 10
        && arr.getProperty(1).toNumber() === 20
        && arr.getProperty(2).toNumber() === 30;
}

export function etsCheckNativeGrefCallback(): boolean {
    let func = nativeGetRef();
    let res = func.invoke(5, 10);  // simulate: func(5, 10)
    return res.toNumber() === 50;
}

export function etsCheckNativeGrefNullSafety(): boolean {
    let value = nativeGetRef();
    return value.isNull() || value.isUndefined();
}


export function etsCheckNestedObject(): boolean {
  const obj = nativeGetRef();
  const tag = obj.getProperty("meta").getProperty("tag").toString();
  const active = obj.getProperty("meta").getProperty("inner").getProperty("active").toBoolean();
  return tag === "nested" && active;
}

export function etsCheckNestedArray(): boolean {
  const arr = nativeGetRef();
  return arr.getProperty(0).getProperty(1).toNumber() === 2 &&
         arr.getProperty(2).getProperty(0).toNumber() === 5;
}

export function etsCheckObjectWithMethod(): boolean {
    let obj = nativeGetRef();
    let result = obj.invokeMethod("getDouble").toNumber();
    return result === 84;
}

export function etsCheckSymbolFieldIgnored(): boolean {
    let obj = nativeGetRef();
    let visible = obj.getProperty("visible").toString();
    return visible === "shown";
}


export function etsCheckUndefinedField(): boolean {
    let obj = nativeGetRef();
    return obj.getProperty("a").toNumber() === 1 && obj.getProperty("b").isUndefined();
}

export function etsCheckEmptyArray(): boolean {
    let arr = nativeGetRef();
    let len = arr.getProperty("length").toNumber();
    return len === 0;
}

export function etsCheckDateObject(): boolean {
    let date = nativeGetRef();
    let dateStr = date.invokeMethod("toString").toString();
    return dateStr.includes("2025");
}