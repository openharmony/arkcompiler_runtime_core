/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

export const ts_number = 1;
export const ts_string = "string";
interface AnyTypeMethod<T> {
  get(a: T): T;
}

export class AnyTypeMethodClass<T> implements AnyTypeMethod<T> {
  get(a: T): T {
    return a;
  }
}

export function create_interface_class_any_type_method(): AnyTypeMethodClass<unknown> {
  return new AnyTypeMethodClass<unknown>();
}

type UnionType = string | number;

interface UnionTypeMethod {
  get(a: UnionType): UnionType;
}

export class UnionTypeMethodClass implements UnionTypeMethod {
  get(a: UnionType): UnionType {
    return a;
  }
}

export function create_interface_class_union_type_method(): AnyTypeMethodClass<number> {
  return new AnyTypeMethodClass<number>();
}

interface ArgInterface {
  get(): number;
}

export function subset_by_ref_interface(obj: ArgInterface): number {
  return obj.get();
}

class UserClass {
  public value = 1;
}

interface SubsetByValue {
  get(): UserClass;
}

export class SubsetByValueClass implements SubsetByValue {
  get(): UserClass {
    return new UserClass();
  }
}

export function create_subset_by_value_class_from_ts(): SubsetByValueClass {
  return new SubsetByValueClass();
}

interface OptionalMethod {
  getNum?(): number;
  getStr(): string;
}

export class WithOptionalMethodClass implements OptionalMethod {
  getNum(): number {
    return ts_number;
  }

  getStr(): string {
    return ts_string;
  }
}

export class WithoutOptionalMethodClass implements OptionalMethod {
  getStr(): string {
    return ts_string;
  }
}

export function create_class_with_optional_method(): WithOptionalMethodClass {
  return new WithOptionalMethodClass();
}

export function create_class_without_optional_method(): WithoutOptionalMethodClass {
  return new WithoutOptionalMethodClass();
}

interface OptionalArgResult {
  with: WithOptionalMethodClass;
  without?: WithoutOptionalMethodClass;
}

export function optional_arg(
  arg: WithOptionalMethodClass,
  optional?: WithoutOptionalMethodClass,
): OptionalArgResult {
  if (optional) {
    return { with: arg, without: optional };
  }

  return { with: arg };
}

type ArrayArg = (WithOptionalMethodClass | WithoutOptionalMethodClass)[];

export function optional_arg_array(...arg: ArrayArg): OptionalArgResult {
  const withOptional = arg[0] as WithOptionalMethodClass;
  const withoutOptional = arg[1] as WithoutOptionalMethodClass;

  if (withoutOptional) {
    return { with: withOptional, without: withoutOptional };
  }

  return { with: withOptional };
}

interface TupleType {
  get(arg: [number, string]): [number, string];
}

export class TupleTypeMethodClass implements TupleType {
  get(arg: [number, string]): [number, string] {
    return arg;
  }
}

export function create_interface_class_tuple_type_method_from_ts(): TupleTypeMethodClass {
  return new TupleTypeMethodClass();
}

export const unionTypeMethodInstanceClass = new UnionTypeMethodClass();
export const subsetByValueInstanceClass = new SubsetByValueClass();
export const tupleInstanceClass = new TupleTypeMethodClass();
export const withoutOptionalMethodInstanceClass =
  new WithoutOptionalMethodClass();
export const withOptionalMethodInstanceClass = new WithOptionalMethodClass();
