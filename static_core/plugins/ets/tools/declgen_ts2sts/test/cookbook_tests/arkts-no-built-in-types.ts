/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

export let sym1: Symbol = Symbol("111");

export const sym2: Symbol = Symbol("description1");

export const Direction = {
  Up: Symbol("Up"),
  Down: Symbol("Down"),
  Left: Symbol("Left"),
  Right: Symbol("Right")
};

export const map = new Map<Symbol, string>();

export const sym5: Symbol = Symbol("key");
export function getSymbol(): Symbol {
  return sym5;
}
export const mySym = getSymbol();

export function printSymbol(value: Symbol): void {
  console.log(value);
}

export type SymbolAlias = symbol;
export const sym6: SymbolAlias = Symbol("description");

export const obj3: { [key: symbol]: string } = {};