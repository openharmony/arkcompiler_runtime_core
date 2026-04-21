/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

import * as ts from 'typescript';
import { Extension } from '../../utils/Extension';
import * as path from 'node:path';

export const JSVALUE: string = 'Any';
export const ESOBJECT: string = 'ESObject';
export const ARRAY_TYPE: string = 'Array';
export const STRING: string = 'string';
export const ETS_KEYWORDS: ReadonlySet<string> = new Set(['Class', 'ESObject', 'MethodType']);

const INCOMPATIBLE_BUILTINS = new Set([
  'Object',
  'Function',
  'Resource',
  'Symbol',
  'SharedArrayBuffer',
  'PropertyDecorator',
  'Transferable',
  'InstanceType',
  'ESObject'
]);

const INCOMPATIBLE_SDKS = new Set<string>(['@ohos.taskpool', '@arkts.collections', '@ohos.worker']);

const STA_RESERVED_KEYWORDS = new Set<string>([
  'abstract',
  'enum',
  'let',
  'this',
  'as',
  'export',
  'native',
  'throw',
  'async',
  'extends',
  'new',
  'true',
  'await',
  'false',
  'null',
  'try',
  'break',
  'final',
  'overload',
  'typeof',
  'case',
  'for',
  'override',
  'undefined',
  'class',
  'function',
  'private',
  'while',
  'const',
  'if',
  'protected',
  'constructor',
  'implements',
  'public',
  'continue',
  'import',
  'return',
  'default',
  'in',
  'static',
  'do',
  'instanceof',
  'switch',
  'else',
  'interface',
  'super'
]);

const STA_HARD_KEYWORDS = new Set<string>([
  'Any',
  'bigint',
  'BigInt',
  'boolean',
  'Boolean',
  'byte',
  'Byte',
  'char',
  'Char',
  'double',
  'Double',
  'float',
  'Float',
  'int',
  'Int',
  'long',
  'Long',
  'number',
  'Number',
  'Object',
  'object',
  'short',
  'Short',
  'string',
  'String',
  'void'
]);

export const FINAL_CLASS: string[] = [
  'ArgumentOutOfRangeError',
  'ArrayIndexOutOfBoundsError',
  'ArrayStoreError',
  'ArrayType',
  'ArrayValue',
  'ArithmeticError',
  'AtomicFlag',
  'Boolean',
  'BooleanType',
  'BooleanValue',
  'Byte',
  'ByteType',
  'ByteValue',
  'CallBodyDefault',
  'CallBodyFunction',
  'CallBodyMethod',
  'CallBodyErasedFunction',
  'Char',
  'CharType',
  'CharValue',
  'Class',
  'ClassCastError',
  'ClassType',
  'ClassValue',
  'Chrono',
  'Console',
  'Coroutine',
  'CoroutineExtras',
  'DebuggerAPI',
  'Double',
  'DoubleType',
  'DoubleValue',
  'EnumConstant',
  'EnumType',
  'Field',
  'FieldCreator',
  'FinalizableWeakRef',
  'FinalizationRegistry',
  'Float',
  'FloatType',
  'FloatValue',
  'GC',
  'IllegalArgumentError',
  'InterfaceType',
  'Int',
  'IntIntValue',
  'IntType',
  'JSError',
  'JSRuntime',
  'JSValue',
  'LambdaType',
  'LambdaValue',
  'Logger',
  'Long',
  'LongType',
  'LongValue',
  'Method',
  'MethodCreator',
  'MethodType',
  'NegativeArraySizeError',
  'NoDataError',
  'NullType',
  'NullValue',
  'Parameter',
  'ParameterCreator',
  'Promise',
  'Runtime',
  'RuntimeError',
  'Short',
  'ShortType',
  'ShortValue',
  'String',
  'StringBuilder',
  'StringIndexOutOfBoundsError',
  'StringType',
  'StringValue',
  'TupleType',
  'Types',
  'UnionCase',
  'UnionType',
  'UnsupportedOperationError',
  'UndefinedType',
  'UndefinedValue',
  'Void',
  'VoidType',
  'VoidValue',
  'WeakRef',
  'Error'
];

export const UTILITY_TYPES = new Map<string, string>([
  ['Capitalize', STRING],
  ['Uncapitalize', STRING],
  ['Uppercase', STRING],
  ['Lowercase', STRING],
  ['ConstructorParameters', ARRAY_TYPE],
  ['Exclude', JSVALUE],
  ['Extract', JSVALUE],
  ['InstanceType', JSVALUE],
  ['NoInfer', JSVALUE],
  ['Omit', JSVALUE],
  ['OmitThisParameter', JSVALUE],
  ['Parameters', ARRAY_TYPE],
  ['Pick', JSVALUE],
  ['ThisParameterType', JSVALUE],
  ['ThisType', JSVALUE]
]);

export enum INTEROP_NAMESPACE {
  Dynamic = 'es',
  Static = 'st'
}
export enum INTEROP_TYPE {
  Array = 'Array',
  Map = 'Map',
  Set = 'Set',
  STValue = 'STValue'
}
export const INTEROP_SDK_NAME = '@ohos.lang.interop';

export function isReservedKeyword(word: string): boolean {
  return STA_RESERVED_KEYWORDS.has(word);
}

export function isHardKeyword(word: string): boolean {
  return STA_HARD_KEYWORDS.has(word);
}

export function isHardOrReservedKeyword(word: string): boolean {
  return isReservedKeyword(word) || isHardKeyword(word);
}

export const LIMIT_DECORATOR: string[] = ['Sendable', 'Concurrent'];

export function isUtilityType(node: ts.TypeReferenceNode): boolean {
  return ts.isIdentifier(node.typeName) && UTILITY_TYPES.has(node.typeName.text);
}

export function createJSValueTypeNode(context: ts.TransformationContext): ts.TypeReferenceNode {
  const identifier = context.factory.createIdentifier(JSVALUE);
  return context.factory.createTypeReferenceNode(identifier, undefined);
}

export function createJSValueArrayTypeNode(context: ts.TransformationContext): ts.ArrayTypeNode {
  const jsValueTypeNode = createJSValueTypeNode(context);
  return context.factory.createArrayTypeNode(jsValueTypeNode);
}

export function createJSValueArrayTypeReferenceNode(context: ts.TransformationContext): ts.TypeReferenceNode {
  const jsValueTypeNode = createJSValueTypeNode(context);
  const arrayIdentifier = context.factory.createIdentifier(ARRAY_TYPE);
  return context.factory.createTypeReferenceNode(arrayIdentifier, [jsValueTypeNode]);
}

export function isIncompatibleBuiltinType(type: ts.TypeReferenceNode): boolean {
  const typeName = type.typeName;
  if (typeName !== undefined && ts.isIdentifier(typeName)) {
    return INCOMPATIBLE_BUILTINS.has(typeName.text);
  }
  return false;
}

export function isIncompatibleSDK(sdkName: string): boolean {
  return INCOMPATIBLE_SDKS.has(sdkName);
}

export function getBaseNameFromFilePath(filePath: string): string {
  if (filePath.endsWith(Extension.DTS)) {
    return path.basename(filePath, Extension.DTS);
  } else if (filePath.endsWith(Extension.DETS)) {
    return path.basename(filePath, Extension.DETS);
  }
  return path.basename(filePath);
}

export function isNodeParentRestParameter(node: ts.Node): boolean {
  if (ts.isTypeReferenceNode(node) || ts.isArrayTypeNode(node) || ts.isTupleTypeNode(node)) {
    const parent = node.parent;
    if (parent && ts.isParameter(parent)) {
      return !!parent.dotDotDotToken;
    }
  }
  return false;
}
