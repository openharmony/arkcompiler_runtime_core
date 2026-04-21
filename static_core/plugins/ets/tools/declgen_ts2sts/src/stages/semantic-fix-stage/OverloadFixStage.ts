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

import * as ts from 'typescript';
import { FaultID } from '../utils/FaultId';
import { StageOptions, TransformationStage } from '../Stage';
import { VisitorTraverser, DepthFirstAlgorithm, TraverserState } from '../Traverser';
import * as typeUtils from '../utils/TypeUtils';

type PrevState = undefined;
type NextState = undefined;
type LocalState = undefined;

export class OverloadFixStage extends TransformationStage<PrevState, NextState, LocalState> {
  constructor(options?: StageOptions) {
    super(
      [OverloadFixTraverser],
      () => undefined,
      () => undefined,
      false,
      options
    );
  }
}

class OverloadFixTraverser extends VisitorTraverser<PrevState, LocalState> {
  constructor(
    context: ts.TransformationContext,
    typeChecker: ts.TypeChecker,
    state: TraverserState<PrevState, LocalState>
  ) {
    super(new DepthFirstAlgorithm(), context, typeChecker, state);
  }

  protected buildVisitors(): Map<ts.SyntaxKind, ts.Visitor[]> {
    return new Map<ts.SyntaxKind, ts.Visitor[]>([
      [
        ts.SyntaxKind.InterfaceDeclaration,
        [this[FaultID.MergeDuplicatedMethods].bind(this), this[FaultID.MergeDuplicatedIndexSignatures].bind(this)]
      ],
      [
        ts.SyntaxKind.ClassDeclaration,
        [
          this[FaultID.MergeDuplicatedConstructors].bind(this),
          this[FaultID.MergeDuplicatedMethods].bind(this),
          this[FaultID.MergeDuplicatedIndexSignatures].bind(this)
        ]
      ],
      [ts.SyntaxKind.SourceFile, [this[FaultID.MergeDuplicatedFunctions].bind(this)]],
      [ts.SyntaxKind.ModuleDeclaration, [this[FaultID.MergeDuplicatedFunctions].bind(this)]]
    ]);
  }

  /**
   * Rule: arkts-merge-duplicated-methods
   */
  private [FaultID.MergeDuplicatedMethods](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isInterfaceDeclaration(node) && !ts.isClassDeclaration(node)) {
      return node;
    }

    const factory = this.context.factory;
    const methods: MethodLike[] = [];
    for (const member of node.members) {
      if (ts.isMethodSignature(member) || ts.isMethodDeclaration(member)) {
        methods.push(member as MethodLike);
      }
    }

    if (methods.length <= 1) {
      return node;
    }

    const replacements = clusterAndMerge(
      methods,
      getClusterKey,
      (group) => mergeMethods(factory, group, this.typeChecker),
      this.typeChecker
    );
    if (ts.isInterfaceDeclaration(node)) {
      const newMembers = rebuildList(node.members, replacements);
      return factory.updateInterfaceDeclaration(
        node,
        node.modifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        newMembers
      );
    } else {
      const newMembers = rebuildList((node as ts.ClassDeclaration).members, replacements);
      return factory.updateClassDeclaration(
        node as ts.ClassDeclaration,
        node.modifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        newMembers
      );
    }
  }

  /**
   * Rule: arkts-merge-duplicated-constructors
   */
  private [FaultID.MergeDuplicatedConstructors](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isClassDeclaration(node)) {
      return node;
    }

    const factory = this.context.factory;
    const constructors: ts.ConstructorDeclaration[] = [];
    for (const member of node.members) {
      if (ts.isConstructorDeclaration(member)) {
        constructors.push(member);
      }
    }

    if (constructors.length <= 1) {
      return node;
    }

    const replacements = clusterAndMerge(
      constructors,
      getConstructorClusterKey,
      (group) => mergeConstructors(factory, group, this.typeChecker),
      this.typeChecker
    );
    const newMembers = rebuildList(node.members, replacements);

    return factory.updateClassDeclaration(
      node,
      node.modifiers,
      node.name,
      node.typeParameters,
      node.heritageClauses,
      newMembers as ts.ClassElement[]
    );
  }

  /**
   * Rule: arkts-merge-duplicated-index-signatures
   *
   * Unlike methods/constructors/functions, all index signatures within a single
   * interface or class are merged into one (no clustering by parameter shape).
   */
  private [FaultID.MergeDuplicatedIndexSignatures](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isInterfaceDeclaration(node) && !ts.isClassDeclaration(node)) {
      return node;
    }

    const factory = this.context.factory;
    const indexSigs: ts.IndexSignatureDeclaration[] = [];
    for (const member of node.members) {
      if (ts.isIndexSignatureDeclaration(member)) {
        indexSigs.push(member);
      }
    }

    if (indexSigs.length <= 1) {
      return node;
    }

    const merged = mergeIndexSignatures(factory, indexSigs, this.typeChecker);
    const replacements = new Map<ts.Node, ts.Node | null>();
    replacements.set(indexSigs[0], merged);
    for (let i = 1; i < indexSigs.length; i++) {
      replacements.set(indexSigs[i], null);
    }

    if (ts.isInterfaceDeclaration(node)) {
      const newMembers = rebuildList(node.members, replacements);
      return factory.updateInterfaceDeclaration(
        node,
        node.modifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        newMembers
      );
    } else {
      const newMembers = rebuildList((node as ts.ClassDeclaration).members, replacements);
      return factory.updateClassDeclaration(
        node as ts.ClassDeclaration,
        node.modifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        newMembers as ts.ClassElement[]
      );
    }
  }

  /**
   * Rule: arkts-merge-duplicated-functions
   */
  private [FaultID.MergeDuplicatedFunctions](node: ts.Node): ts.VisitResult<ts.Node> {
    let statements: readonly ts.Statement[];
    if (ts.isSourceFile(node)) {
      statements = node.statements;
    } else if (ts.isModuleDeclaration(node) && node.body && ts.isModuleBlock(node.body)) {
      statements = node.body.statements;
    } else {
      return node;
    }

    const factory = this.context.factory;
    const functions: ts.FunctionDeclaration[] = [];
    for (const stmt of statements) {
      if (ts.isFunctionDeclaration(stmt)) {
        functions.push(stmt);
      }
    }

    if (functions.length <= 1) {
      return node;
    }

    const replacements = clusterAndMerge(
      functions,
      getFunctionClusterKey,
      (group) => mergeFunctionDecls(factory, group, this.typeChecker),
      this.typeChecker
    );
    const newStatements = rebuildList(statements, replacements);

    if (ts.isSourceFile(node)) {
      return factory.updateSourceFile(node, newStatements);
    } else {
      const newBody = factory.updateModuleBlock(node.body as ts.ModuleBlock, newStatements);
      return factory.updateModuleDeclaration(node, node.modifiers, node.name, newBody);
    }
  }
}

type MethodLike = ts.MethodSignature | ts.MethodDeclaration;
type DeclarationWithReturnType = { readonly type?: ts.TypeNode };
type DeclarationWithTypeParams = { readonly typeParameters?: ts.NodeArray<ts.TypeParameterDeclaration> };

const WILDCARD = '*';
interface HasParameters {
  readonly parameters: ts.NodeArray<ts.ParameterDeclaration>;
}

/**
 * Check if a type node specifically represents a type parameter (T, U, etc.).
 */
function isTypeParameterType(typeNode: ts.TypeNode, typeChecker: ts.TypeChecker): boolean {
  const type = typeChecker.getTypeAtLocation(typeNode);
  return !!(type.flags & ts.TypeFlags.TypeParameter);
}

/**
 * Get a qualified name for a type symbol that includes source file path,
 * so that same-named types from different files are distinguished.
 */
function getSymbolAssemblyName(symbol: ts.Symbol): string {
  const decls = symbol.declarations;
  if (!decls || decls.length === 0) {
    return symbol.name;
  }
  const sf = decls[0].getSourceFile();
  return `${sf.fileName}#${symbol.name}`;
}

/**
 * Flatten nested union types into a flat array of member type nodes.
 */
function flattenUnionTypes(typeNode: ts.TypeNode): ts.TypeNode[] {
  if (ts.isUnionTypeNode(typeNode)) {
    const result: ts.TypeNode[] = [];
    for (const t of typeNode.types) {
      result.push(...flattenUnionTypes(t));
    }
    return result;
  }
  if (ts.isParenthesizedTypeNode(typeNode)) {
    return flattenUnionTypes(typeNode.type);
  }
  return [typeNode];
}

/**
 * Strip 'readonly ' prefix from an assembly type string.
 */
function stripReadonly(asmType: string): string {
  return asmType.startsWith('readonly ') ? asmType.slice('readonly '.length) : asmType;
}

/**
 * Normalize a union type node into an assembly type string.
 * Rules:
 * - Linearize nested unions
 * - Remove never types
 * - If any member is wildcard (*), entire union is *
 * - Merge members with same base assembly type (readonly/non-readonly merge, keeping readonly)
 * - Sort and join remaining assembly types
 */
function normalizeUnionAssemblyType(unionNode: ts.UnionTypeNode, typeChecker: ts.TypeChecker): string {
  const flatTypes = flattenUnionTypes(unionNode);

  const members = flatTypes.map((t) => ({
    typeNode: t,
    asmType: getAssemblyType(t, typeChecker)
  }));

  const nonNever = members.filter((m) => m.asmType !== 'never' && m.asmType !== 'undefined');
  if (nonNever.length === 0) {
    return 'never';
  }

  if (nonNever.some((m) => m.asmType === WILDCARD)) {
    return WILDCARD;
  }

  // Merge same assembly types; readonly wins over non-readonly
  const seen = new Map<string, string>();
  for (const m of nonNever) {
    const base = stripReadonly(m.asmType);
    const existing = seen.get(base);
    if (!existing) {
      seen.set(base, m.asmType);
    } else if (m.asmType.startsWith('readonly ') && !existing.startsWith('readonly ')) {
      seen.set(base, m.asmType);
    }
  }

  const sorted = [...seen.values()].sort();
  if (sorted.length === 1) {
    return sorted[0];
  }
  return sorted.join('|');
}

/**
 * Compute the assembly type of a resolved ts.Type for duplicate detection.
 * Used when we only have a ts.Type (e.g. after alias resolution) rather than a TypeNode.
 * Mirrors the normalization rules of getAssemblyType/normalizeUnionAssemblyType.
 */
function getAssemblyTypeFromCheckerType(type: ts.Type, typeChecker: ts.TypeChecker): string {
  if (type.isUnion()) {
    const members = type.types
      .map((t) => getAssemblyTypeFromCheckerType(t, typeChecker))
      .filter((s) => s !== 'never' && s !== 'undefined');
    if (members.length === 0) {
      return 'never';
    }
    if (members.some((s) => s === WILDCARD)) {
      return WILDCARD;
    }
    // Merge same base types (readonly vs non-readonly)
    const seen = new Map<string, string>();
    for (const m of members) {
      const base = stripReadonly(m);
      const existing = seen.get(base);
      if (!existing) {
        seen.set(base, m);
      } else if (m.startsWith('readonly ') && !existing.startsWith('readonly ')) {
        seen.set(base, m);
      }
    }
    const sorted = [...seen.values()].sort();
    return sorted.length === 1 ? sorted[0] : sorted.join('|');
  }

  if (type.symbol) {
    return getSymbolAssemblyName(type.symbol);
  }
  return typeChecker.typeToString(type, undefined, ts.TypeFormatFlags.NoTruncation);
}

/**
 * Compute the assembly type of a TypeNode for duplicate detection.
 *
 * Assembly type rules:
 * - any, Any (JSVALUE), and type parameters → '*' (wildcard)
 * - Type aliases → resolved to underlying type via TypeChecker
 * - Literal types (e.g. "hello", 1, true) → corresponding primitive (string, number, boolean)
 * - Array syntax T[] → resolved via TypeChecker symbol (same as Array<T>)
 * - Tuple [X,Y] → 'Tuple|elementCount'
 * - Generic types: ignore type arguments (e.g. Set<string> → Set symbol name)
 * - readonly modifier preserved in assembly type
 * - Types from different files get different assembly types (file-qualified symbol name)
 * - Union types: linearized, normalized, merged by assembly type
 */
/**
 * Try to resolve a TypeNode whose assembly type is a wildcard ('*').
 * Returns the wildcard string if matched, otherwise undefined.
 */
function tryGetWildcardAssemblyType(typeNode: ts.TypeNode, type: ts.Type): string | undefined {
  if (typeNode.kind === ts.SyntaxKind.AnyKeyword) {
    return WILDCARD;
  }
  if (
    ts.isTypeReferenceNode(typeNode) &&
    ts.isIdentifier(typeNode.typeName) &&
    typeNode.typeName.text === typeUtils.JSVALUE
  ) {
    return WILDCARD;
  }
  if (type.flags & ts.TypeFlags.TypeParameter) {
    return WILDCARD;
  }
  if (type.flags & ts.TypeFlags.Any) {
    return WILDCARD;
  }
  return undefined;
}

/**
 * Compute the assembly type for a literal type node (e.g. "hello", 1, true, null).
 */
function getLiteralAssemblyType(typeNode: ts.LiteralTypeNode, type: ts.Type, typeChecker: ts.TypeChecker): string {
  const lit = typeNode.literal;
  if (ts.isStringLiteral(lit)) {
    return 'string';
  }
  if (ts.isNumericLiteral(lit)) {
    return 'number';
  }
  if (lit.kind === ts.SyntaxKind.TrueKeyword || lit.kind === ts.SyntaxKind.FalseKeyword) {
    return 'boolean';
  }
  if (lit.kind === ts.SyntaxKind.NullKeyword) {
    return 'null';
  }
  if (ts.isPrefixUnaryExpression(lit)) {
    return 'number';
  }
  return typeChecker.typeToString(type, undefined, ts.TypeFormatFlags.NoTruncation);
}

/**
 * Compute the assembly type of a TypeNode for duplicate detection.
 *
 * Assembly type rules:
 * - any, Any (JSVALUE), and type parameters → '*' (wildcard)
 * - Type aliases → resolved to underlying type via TypeChecker
 * - Literal types (e.g. "hello", 1, true) → corresponding primitive (string, number, boolean)
 * - Array syntax T[] → resolved via TypeChecker symbol (same as Array<T>)
 * - Tuple [X,Y] → 'Tuple|elementCount'
 * - Generic types: ignore type arguments (e.g. Set<string> → Set symbol name)
 * - readonly modifier preserved in assembly type
 * - Types from different files get different assembly types (file-qualified symbol name)
 * - Union types: linearized, normalized, merged by assembly type
 */
function getAssemblyType(typeNode: ts.TypeNode | undefined, typeChecker: ts.TypeChecker): string {
  if (!typeNode) {
    return WILDCARD;
  }

  // readonly T → 'readonly <asm(T)>'
  if (ts.isTypeOperatorNode(typeNode) && typeNode.operator === ts.SyntaxKind.ReadonlyKeyword) {
    const inner = getAssemblyType(typeNode.type, typeChecker);
    return inner === WILDCARD ? WILDCARD : `readonly ${inner}`;
  }

  // Parenthesized → unwrap
  if (ts.isParenthesizedTypeNode(typeNode)) {
    return getAssemblyType(typeNode.type, typeChecker);
  }

  // Keyword types (other than `any`, handled below as wildcard)
  if (typeNode.kind !== ts.SyntaxKind.AnyKeyword) {
    const keyword = ts.tokenToString(typeNode.kind);
    if (keyword !== undefined) {
      return keyword;
    }
  }

  const type = typeChecker.getTypeAtLocation(typeNode);

  // any, Any (JSVALUE), and type parameters → wildcard
  const wildcard = tryGetWildcardAssemblyType(typeNode, type);
  if (wildcard !== undefined) {
    return wildcard;
  }

  // Literal types → corresponding primitive type
  if (ts.isLiteralTypeNode(typeNode)) {
    return getLiteralAssemblyType(typeNode, type, typeChecker);
  }

  // Array syntax: T[] — resolve via TypeChecker for consistency with Array<T>
  if (ts.isArrayTypeNode(typeNode)) {
    return type.symbol ? getSymbolAssemblyName(type.symbol) : 'Array';
  }

  // Tuple: [X, Y]
  if (ts.isTupleTypeNode(typeNode)) {
    return `Tuple<${typeNode.elements.length}>`;
  }

  // Union type: normalize
  if (ts.isUnionTypeNode(typeNode)) {
    return normalizeUnionAssemblyType(typeNode, typeChecker);
  }

  // Type reference (class, interface, type alias, generic class, etc.)
  // TypeChecker resolves aliases to underlying type; type arguments are ignored via symbol name
  if (ts.isTypeReferenceNode(typeNode)) {
    if (type.symbol) {
      return getSymbolAssemblyName(type.symbol);
    }
    // No symbol means the alias resolved to a structural type (e.g. type A = number | string).
    // Normalize via the resolved ts.Type so the result matches normalizeUnionAssemblyType.
    return getAssemblyTypeFromCheckerType(type, typeChecker);
  }

  // Fallback: function types, mapped types, conditional types, etc.
  return typeChecker.typeToString(type, undefined, ts.TypeFormatFlags.NoTruncation);
}

/**
 * Check if two parameter lists are "duplicated" (same assembly types ignoring optionality).
 */
function areParametersDuplicated(
  a: ts.NodeArray<ts.ParameterDeclaration>,
  b: ts.NodeArray<ts.ParameterDeclaration>,
  typeChecker: ts.TypeChecker
): boolean {
  if (a.length !== b.length) {
    return false;
  }
  for (let i = 0; i < a.length; i++) {
    if (getAssemblyType(a[i].type, typeChecker) !== getAssemblyType(b[i].type, typeChecker)) {
      return false;
    }
  }
  return true;
}

/**
 * Rebuild a list by applying a replacement map: items mapped to a replacement
 * are substituted at their first occurrence; items mapped to null are removed.
 * Items not in the map are kept as-is.
 */
function rebuildList<N extends ts.Node>(
  original: readonly N[] | ts.NodeArray<N>,
  replacements: Map<ts.Node, ts.Node | null>
): N[] {
  const result: N[] = [];
  for (const item of original) {
    if (!replacements.has(item)) {
      result.push(item);
      continue;
    }
    const replacement = replacements.get(item)!;
    if (replacement !== null) {
      result.push(replacement as N);
    }
    // null → item was absorbed into another merge group, skip it
  }
  return result;
}

/**
 * Within a cluster of declarations, find groups where parameter types are duplicated.
 */
function findDuplicateGroups<T extends HasParameters>(cluster: T[], typeChecker: ts.TypeChecker): T[][] {
  const groups: T[][] = [];
  const used = new Set<number>();
  for (let i = 0; i < cluster.length; i++) {
    if (used.has(i)) {
      continue;
    }
    const group = [cluster[i]];
    used.add(i);
    for (let j = i + 1; j < cluster.length; j++) {
      if (used.has(j)) {
        continue;
      }
      if (areParametersDuplicated(cluster[i].parameters, cluster[j].parameters, typeChecker)) {
        group.push(cluster[j]);
        used.add(j);
      }
    }
    groups.push(group);
  }
  return groups;
}

/**
 * Cluster items by a key function, then within each cluster find groups of
 * parameter-duplicated items and merge them.
 * Returns a Map from original item → merged result (for group leader) or null (for absorbed items).
 */
function clusterAndMerge<T extends ts.Node & HasParameters>(
  items: T[],
  getKey: (item: T) => string | undefined,
  merge: (group: T[]) => T,
  typeChecker: ts.TypeChecker
): Map<ts.Node, ts.Node | null> {
  const clusters = new Map<string, T[]>();
  const clusterOrder: string[] = [];

  for (const item of items) {
    const key = getKey(item);
    if (!key) {
      continue;
    }
    if (!clusters.has(key)) {
      clusters.set(key, []);
      clusterOrder.push(key);
    }
    clusters.get(key)!.push(item);
  }

  const replacements = new Map<ts.Node, ts.Node | null>();
  for (const key of clusterOrder) {
    const cluster = clusters.get(key)!;
    if (cluster.length === 1) {
      continue;
    }
    for (const group of findDuplicateGroups(cluster, typeChecker)) {
      if (group.length === 1) {
        continue;
      }
      const merged = merge(group);
      replacements.set(group[0], merged); // leader → merged result
      for (let i = 1; i < group.length; i++) {
        replacements.set(group[i], null); // others → absorbed
      }
    }
  }

  return replacements;
}

/**
 * Key for clustering overload methods.
 * Methods with the same clusterKey are candidates for merging.
 */
function getClusterKey(method: MethodLike): string | undefined {
  const name = method.name ? method.name.getText() : undefined;
  if (!name) {
    return undefined;
  }
  const isStatic = ts.canHaveModifiers(method)
    ? (ts.getModifiers(method) ?? []).some((m) => m.kind === ts.SyntaxKind.StaticKeyword)
    : false;
  const paramCount = method.parameters.length;
  return `${name}|${isStatic ? 'static' : 'instance'}|${paramCount}`;
}

/**
 * Check if a TypeNode is `void`.
 */
function isVoidType(typeNode: ts.TypeNode): boolean {
  return typeNode.kind === ts.SyntaxKind.VoidKeyword;
}

const printer = ts.createPrinter({ removeComments: true });

/**
 * Get the text representation for deduplication of types in union.
 * Uses printer for synthesized nodes that lack source file context.
 */
function getTypeTextForDedup(typeNode: ts.TypeNode): string {
  try {
    return typeNode.getText();
  } catch {
    return printer.printNode(ts.EmitHint.Unspecified, typeNode, ts.createSourceFile('', '', ts.ScriptTarget.Latest));
  }
}

/**
 * Remove duplicate types from an array of TypeNodes by text.
 */
function removeDuplicateTypes(types: ts.TypeNode[]): ts.TypeNode[] {
  const seen = new Set<string>();
  const result: ts.TypeNode[] = [];
  for (const t of types) {
    const text = getTypeTextForDedup(t);
    if (!seen.has(text)) {
      seen.add(text);
      result.push(t);
    }
  }
  return result;
}

/**
 * Merge return types of duplicated methods into a union type.
 * void → undefined in the union.
 */
function mergeReturnTypes(factory: ts.NodeFactory, methods: DeclarationWithReturnType[]): ts.TypeNode {
  const returnTypes: ts.TypeNode[] = [];
  for (const m of methods) {
    const rt = m.type;
    if (!rt) {
      returnTypes.push(factory.createKeywordTypeNode(ts.SyntaxKind.AnyKeyword));
    } else if (isVoidType(rt)) {
      returnTypes.push(factory.createKeywordTypeNode(ts.SyntaxKind.UndefinedKeyword));
    } else if (ts.isUnionTypeNode(rt)) {
      for (const t of rt.types) {
        returnTypes.push(isVoidType(t) ? factory.createKeywordTypeNode(ts.SyntaxKind.UndefinedKeyword) : t);
      }
    } else {
      returnTypes.push(rt);
    }
  }

  const deduped = removeDuplicateTypes(returnTypes);
  if (deduped.length === 1) {
    return deduped[0];
  }
  return factory.createUnionTypeNode(deduped);
}

/**
 * Merge a single parameter across duplicated methods.
 * Detects rest parameters and delegates to appropriate merge strategy.
 */
function mergeParameter(
  factory: ts.NodeFactory,
  params: ts.ParameterDeclaration[],
  typeChecker: ts.TypeChecker,
  index: number
): ts.ParameterDeclaration {
  const isRest = params.some((p) => p.dotDotDotToken !== undefined);
  if (isRest) {
    return mergeRestParameter(factory, params);
  }
  return mergeRegularParameter(factory, params, typeChecker, index);
}

/**
 * Merge rest parameters (...args).
 * - Same type → keep one
 * - Different types → Any[]
 * - Different identifiers → ...args
 */
function mergeRestParameter(factory: ts.NodeFactory, params: ts.ParameterDeclaration[]): ts.ParameterDeclaration {
  const first = params[0];

  const firstName = ts.isIdentifier(first.name) ? first.name.text : '';
  const allSameName = params.every((p) => ts.isIdentifier(p.name) && p.name.text === firstName);
  const mergedName = allSameName ? first.name : factory.createIdentifier('args');

  const typeNodes = params.map((p) => p.type).filter((t): t is ts.TypeNode => t !== undefined);
  const firstText = typeNodes.length > 0 ? getTypeTextForDedup(typeNodes[0]) : '';
  const allSameType = typeNodes.length > 0 && typeNodes.every((t) => getTypeTextForDedup(t) === firstText);

  let mergedType: ts.TypeNode;
  if (allSameType && typeNodes.length > 0) {
    mergedType = typeNodes[0];
  } else {
    mergedType = factory.createTypeReferenceNode(factory.createIdentifier(typeUtils.ARRAY_TYPE), [
      factory.createTypeReferenceNode(typeUtils.JSVALUE, undefined)
    ]);
  }

  return factory.updateParameterDeclaration(
    first,
    first.modifiers,
    factory.createToken(ts.SyntaxKind.DotDotDotToken),
    mergedName,
    undefined,
    mergedType,
    undefined
  );
}

/**
 * Merge a regular (non-rest) parameter across duplicated methods.
 * - All wildcards (any/Any/TypeParam): prefer type param over any/Any
 * - All resolve to same real type: keep textually distinct representations as X | Xa union
 * - Generic classes, arrays, tuples: keep all textually distinct forms as union
 */
function mergeRegularParameter(
  factory: ts.NodeFactory,
  params: ts.ParameterDeclaration[],
  typeChecker: ts.TypeChecker,
  index: number
): ts.ParameterDeclaration {
  const first = params[0];
  const isOptional = params.some((p) => p.questionToken !== undefined);

  const firstName = ts.isIdentifier(first.name) ? first.name.text : '';
  const allSameName = params.every((p) => ts.isIdentifier(p.name) && p.name.text === firstName);
  const mergedName = allSameName ? first.name : factory.createIdentifier(`param${index}`);

  const isWildcard = getAssemblyType(first.type, typeChecker) === WILDCARD;
  let mergedType: ts.TypeNode;

  if (isWildcard) {
    const typeParamNodes: ts.TypeNode[] = [];
    let hasAny = false;
    for (const p of params) {
      if (p.type && isTypeParameterType(p.type, typeChecker)) {
        typeParamNodes.push(p.type);
      } else {
        hasAny = true;
      }
    }
    if (typeParamNodes.length === 0) {
      mergedType = factory.createTypeReferenceNode(typeUtils.JSVALUE, undefined);
    } else {
      const deduped = removeDuplicateTypes(typeParamNodes);
      if (hasAny) {
        deduped.push(factory.createTypeReferenceNode(typeUtils.JSVALUE, undefined));
      }
      mergedType = deduped.length === 1 ? deduped[0] : factory.createUnionTypeNode(deduped);
    }
  } else {
    const typeNodes = params.map((p) => p.type).filter((t): t is ts.TypeNode => t !== undefined);
    const deduped = removeDuplicateTypes(typeNodes);
    mergedType = deduped.length === 1 ? deduped[0] : factory.createUnionTypeNode(deduped);
  }

  return factory.updateParameterDeclaration(
    first,
    first.modifiers,
    first.dotDotDotToken,
    mergedName,
    isOptional ? (first.questionToken ?? factory.createToken(ts.SyntaxKind.QuestionToken)) : undefined,
    mergedType,
    undefined
  );
}

/**
 * Merge type parameter lists from all methods in a duplicate group.
 * Collects all unique type parameters by name across all methods.
 */
function mergeTypeParameters(
  factory: ts.NodeFactory,
  methods: DeclarationWithTypeParams[]
): ts.NodeArray<ts.TypeParameterDeclaration> | undefined {
  const seen = new Set<string>();
  const merged: ts.TypeParameterDeclaration[] = [];
  for (const m of methods) {
    if (!m.typeParameters) {
      continue;
    }
    for (const tp of m.typeParameters) {
      const name = tp.name.text;
      if (!seen.has(name)) {
        seen.add(name);
        merged.push(tp);
      }
    }
  }
  return merged.length > 0 ? factory.createNodeArray(merged) : undefined;
}

/**
 * Merge a cluster of duplicated methods into one.
 */
function mergeMethods(factory: ts.NodeFactory, methods: MethodLike[], typeChecker: ts.TypeChecker): MethodLike {
  if (methods.length === 1) {
    return methods[0];
  }

  const first = methods[0];
  const paramCount = first.parameters.length;

  const mergedParams: ts.ParameterDeclaration[] = [];
  for (let i = 0; i < paramCount; i++) {
    mergedParams.push(
      mergeParameter(
        factory,
        methods.map((m) => m.parameters[i]),
        typeChecker,
        i
      )
    );
  }

  const mergedReturnType = mergeReturnTypes(factory, methods);
  const mergedTypeParams = mergeTypeParameters(factory, methods);

  if (ts.isMethodSignature(first)) {
    return factory.updateMethodSignature(
      first,
      first.modifiers,
      first.name,
      first.questionToken,
      mergedTypeParams,
      factory.createNodeArray(mergedParams),
      mergedReturnType
    );
  } else {
    return factory.updateMethodDeclaration(
      first,
      first.modifiers,
      first.asteriskToken,
      first.name,
      first.questionToken,
      mergedTypeParams,
      mergedParams,
      mergedReturnType,
      undefined
    );
  }
}

/**
 * Cluster key for function declarations: name + param count.
 */
function getFunctionClusterKey(func: ts.FunctionDeclaration): string | undefined {
  const name = func.name ? func.name.getText() : undefined;
  if (!name) {
    return undefined;
  }
  return `${name}|${func.parameters.length}`;
}

/**
 * Cluster key for constructors: param count (constructors have no name distinction).
 */
function getConstructorClusterKey(ctor: ts.ConstructorDeclaration): string {
  return String(ctor.parameters.length);
}

/**
 * Merge a group of duplicated constructors into one.
 */
function mergeConstructors(
  factory: ts.NodeFactory,
  ctors: ts.ConstructorDeclaration[],
  typeChecker: ts.TypeChecker
): ts.ConstructorDeclaration {
  if (ctors.length === 1) {
    return ctors[0];
  }

  const first = ctors[0];
  const paramCount = first.parameters.length;

  const mergedParams: ts.ParameterDeclaration[] = [];
  for (let i = 0; i < paramCount; i++) {
    mergedParams.push(
      mergeParameter(
        factory,
        ctors.map((c) => c.parameters[i]),
        typeChecker,
        i
      )
    );
  }

  return factory.updateConstructorDeclaration(first, first.modifiers, mergedParams, undefined);
}

/**
 * Merge a group of duplicated function declarations into one.
 */
function mergeFunctionDecls(
  factory: ts.NodeFactory,
  funcs: ts.FunctionDeclaration[],
  typeChecker: ts.TypeChecker
): ts.FunctionDeclaration {
  if (funcs.length === 1) {
    return funcs[0];
  }

  const first = funcs[0];
  const paramCount = first.parameters.length;

  const mergedParams: ts.ParameterDeclaration[] = [];
  for (let i = 0; i < paramCount; i++) {
    mergedParams.push(
      mergeParameter(
        factory,
        funcs.map((f) => f.parameters[i]),
        typeChecker,
        i
      )
    );
  }

  const mergedReturnType = mergeReturnTypes(factory, funcs);
  const mergedTypeParams = mergeTypeParameters(factory, funcs);

  return factory.updateFunctionDeclaration(
    first,
    first.modifiers,
    first.asteriskToken,
    first.name,
    mergedTypeParams,
    mergedParams,
    mergedReturnType,
    undefined
  );
}

/**
 * Merge all IndexSignatureDeclarations in a container into a single one.
 * - Key parameter type: union of all distinct key types (by assembly type)
 * - Key parameter name: kept if all signatures agree, otherwise 'key'
 * - Value type: merged via mergeReturnTypes (void → undefined, dedup, union)
 * - readonly: only kept when ALL signatures are readonly (scheme B)
 * - Other modifiers: taken from the first signature
 */
function mergeIndexSignatures(
  factory: ts.NodeFactory,
  sigs: ts.IndexSignatureDeclaration[],
  typeChecker: ts.TypeChecker
): ts.IndexSignatureDeclaration {
  if (sigs.length === 1) {
    return sigs[0];
  }

  const first = sigs[0];

  // Merge key parameter type: dedup by assembly type, preserve original TypeNodes.
  const keyTypeNodes: ts.TypeNode[] = [];
  const seenKeyAsm = new Set<string>();
  for (const sig of sigs) {
    const kp = sig.parameters[0];
    if (!kp || !kp.type) {
      continue;
    }
    const asm = getAssemblyType(kp.type, typeChecker);
    if (!seenKeyAsm.has(asm)) {
      seenKeyAsm.add(asm);
      keyTypeNodes.push(kp.type);
    }
  }
  const mergedKeyType: ts.TypeNode =
    keyTypeNodes.length === 0
      ? factory.createKeywordTypeNode(ts.SyntaxKind.StringKeyword)
      : keyTypeNodes.length === 1
        ? keyTypeNodes[0]
        : factory.createUnionTypeNode(keyTypeNodes);

  // Key parameter name: keep if all match, else 'key'.
  const firstKeyParam = first.parameters[0];
  const firstKeyName = firstKeyParam && ts.isIdentifier(firstKeyParam.name) ? firstKeyParam.name.text : '';
  const allSameKeyName = sigs.every((s) => {
    const p = s.parameters[0];
    return p && ts.isIdentifier(p.name) && p.name.text === firstKeyName;
  });
  const mergedKeyName = allSameKeyName && firstKeyParam ? firstKeyParam.name : factory.createIdentifier('key');

  const mergedKeyParam = factory.createParameterDeclaration(
    undefined,
    undefined,
    mergedKeyName,
    undefined,
    mergedKeyType,
    undefined
  );

  // Merge value type using existing return-type merging logic.
  const mergedValueType = mergeReturnTypes(factory, sigs);

  // readonly: scheme B — only kept when ALL signatures are readonly.
  const allReadonly = sigs.every((s) =>
    (ts.getModifiers(s) ?? []).some((m) => m.kind === ts.SyntaxKind.ReadonlyKeyword)
  );
  const firstModifiers = ts.getModifiers(first) ?? [];
  const nonReadonlyFirstModifiers = firstModifiers.filter((m) => m.kind !== ts.SyntaxKind.ReadonlyKeyword);
  const mergedModifiers: ts.Modifier[] = allReadonly
    ? [...nonReadonlyFirstModifiers, factory.createModifier(ts.SyntaxKind.ReadonlyKeyword)]
    : nonReadonlyFirstModifiers;

  return factory.updateIndexSignature(
    first,
    mergedModifiers.length > 0 ? mergedModifiers : undefined,
    [mergedKeyParam],
    mergedValueType
  );
}
