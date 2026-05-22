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
import * as path from 'path';
import { FaultID } from '../utils/FaultId';
import { StageOptions, TransformationStage } from '../Stage';
import { VisitorTraverser, DepthFirstAlgorithm, TraverserState } from '../Traverser';
import { Extension } from '../../utils/Extension';
import * as typeUtils from '../utils/TypeUtils';

type PrevState = undefined;
type NextState = undefined;
type LocalState = undefined;

export class InteropTypesConversionStage extends TransformationStage<PrevState, NextState, LocalState> {
  constructor(options?: StageOptions) {
    super(
      [InteropTypesConversionTraverser],
      () => {
        return undefined;
      },
      () => undefined,
      false,
      options
    );
  }
}

class InteropTypesConversionTraverser extends VisitorTraverser<PrevState, LocalState> {
  constructor(
    context: ts.TransformationContext,
    typeChecker: ts.TypeChecker,
    state: TraverserState<PrevState, LocalState>
  ) {
    super(new DepthFirstAlgorithm(), context, typeChecker, state);
  }

  protected buildVisitors(): Map<ts.SyntaxKind, ts.Visitor[]> {
    return new Map<ts.SyntaxKind, ts.Visitor[]>([
      [ts.SyntaxKind.TypeReference, [this[FaultID.ConvertESTypeToInteropType].bind(this)]],
      [ts.SyntaxKind.ArrayType, [this[FaultID.ConvertESTypeToInteropType].bind(this)]],
      [ts.SyntaxKind.SourceFile, [this[FaultID.FixInteropImports].bind(this)]]
    ]);
  }

  /**
   * Rule: `arkts-convert-estype-to-interop-type`
   */
  private [FaultID.ConvertESTypeToInteropType](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isTypeReferenceNode(node) && !typeUtils.isNodeParentRestParameter(node)) {
      if (ts.isIdentifier(node.typeName) && isESBuildinType(node)) {
        const interopName = convertESBuiltinNameToInteropName(node.typeName, this.context);
        return this.context.factory.updateTypeReferenceNode(node, interopName, node.typeArguments);
      } else if (ts.isQualifiedName(node.typeName) && isSTType(node, this.typeChecker)) {
        const esBuiltinName = convertSTTypeNameToESBuiltinName(node.typeName, this.context);
        return this.context.factory.updateTypeReferenceNode(node, esBuiltinName, node.typeArguments);
      }
    } else if (ts.isArrayTypeNode(node) && !typeUtils.isNodeParentRestParameter(node)) {
      return createESArrayTypeNode(node.elementType, this.context);
    }
    return node;
  }

  /**
   * Rule: `arkts-fix-interop-imports`
   */
  private [FaultID.FixInteropImports](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isSourceFile(node)) {
      return node;
    }

    const nonInteropImportDeclarationStatements = node.statements.filter((stmt: ts.Statement): boolean => {
      return !ts.isImportDeclaration(stmt) || isNonInteropImportDeclaration(stmt, this.typeChecker);
    });

    const interopImportDeclaration = createESInteropImportDeclaration(this.context);

    const newStatements = [...nonInteropImportDeclarationStatements, interopImportDeclaration];
    return this.context.factory.updateSourceFile(
      node,
      newStatements,
      node.isDeclarationFile,
      node.referencedFiles,
      node.typeReferenceDirectives,
      node.hasNoDefaultLib,
      node.libReferenceDirectives
    );
  }
}

function createESQualifiedName(
  namespace: typeUtils.INTEROP_NAMESPACE,
  type: typeUtils.INTEROP_TYPE,
  context: ts.TransformationContext
): ts.QualifiedName {
  return context.factory.createQualifiedName(
    context.factory.createIdentifier(namespace),
    context.factory.createIdentifier(type)
  );
}

function isESBuildinType(node: ts.TypeReferenceNode): boolean {
  if (ts.isIdentifier(node.typeName)) {
    const typeName = node.typeName.text;
    return (
      typeName === typeUtils.INTEROP_TYPE.Array ||
      typeName === typeUtils.INTEROP_TYPE.Map ||
      typeName === typeUtils.INTEROP_TYPE.Set
    );
  }
  return false;
}

function getBaseNameFromFilePath(filePath: string): string {
  if (filePath.endsWith(Extension.DTS)) {
    return path.basename(filePath, Extension.DTS);
  } else if (filePath.endsWith(Extension.DETS)) {
    return path.basename(filePath, Extension.DETS);
  }
  return path.basename(filePath);
}

function isSTType(node: ts.TypeReferenceNode, typeChecker: ts.TypeChecker): boolean {
  const typeName = node.typeName;
  if (!ts.isQualifiedName(typeName)) {
    return false;
  }
  const type = typeChecker.getTypeAtLocation(typeName);
  const symbol = type.getSymbol();

  if (!symbol) {
    return false;
  }

  const declarations = symbol.getDeclarations();
  if (!declarations || declarations.length === 0) {
    return false;
  }

  for (const decl of declarations) {
    const sourceFileName = decl.getSourceFile().fileName;
    const sdkName = getBaseNameFromFilePath(sourceFileName);
    if (sdkName !== typeUtils.INTEROP_SDK_NAME) {
      continue;
    }
    return true;
  }

  return false;
}

function convertESBuiltinNameToInteropName(
  identifier: ts.Identifier,
  context: ts.TransformationContext
): ts.EntityName {
  let converted: ts.EntityName = context.factory.createIdentifier(typeUtils.JSVALUE);
  switch (identifier.text) {
    case typeUtils.INTEROP_TYPE.Array:
      converted = createESQualifiedName(typeUtils.INTEROP_NAMESPACE.Dynamic, typeUtils.INTEROP_TYPE.Array, context);
      break;
    case typeUtils.INTEROP_TYPE.Map:
      converted = createESQualifiedName(typeUtils.INTEROP_NAMESPACE.Dynamic, typeUtils.INTEROP_TYPE.Map, context);
      break;
    case typeUtils.INTEROP_TYPE.Set:
      converted = createESQualifiedName(typeUtils.INTEROP_NAMESPACE.Dynamic, typeUtils.INTEROP_TYPE.Set, context);
      break;
  }
  return converted;
}

function convertSTTypeNameToESBuiltinName(node: ts.QualifiedName, context: ts.TransformationContext): ts.Identifier {
  let converted: ts.EntityName = context.factory.createIdentifier(typeUtils.JSVALUE);
  if (ts.isIdentifier(node.left)) {
    switch (node.right.text) {
      case typeUtils.INTEROP_TYPE.Array:
        converted = context.factory.createIdentifier(typeUtils.INTEROP_TYPE.Array);
        break;
      case typeUtils.INTEROP_TYPE.Map:
        converted = context.factory.createIdentifier(typeUtils.INTEROP_TYPE.Map);
        break;
      case typeUtils.INTEROP_TYPE.Set:
        converted = context.factory.createIdentifier(typeUtils.INTEROP_TYPE.Set);
        break;
    }
  }
  return converted;
}

function createESArrayTypeNode(
  elementType: ts.TypeNode | ts.TypeNode[],
  context: ts.TransformationContext
): ts.TypeReferenceNode {
  const typeName = context.factory.createQualifiedName(
    context.factory.createIdentifier(typeUtils.INTEROP_NAMESPACE.Dynamic),
    context.factory.createIdentifier(typeUtils.INTEROP_TYPE.Array)
  );

  const typeArguments = Array.isArray(elementType) ? elementType : [elementType];

  return context.factory.createTypeReferenceNode(typeName, typeArguments);
}

function isNonInteropImportDeclaration(node: ts.ImportDeclaration, typeChecker: ts.TypeChecker): boolean {
  const moduleSpecifier = node.moduleSpecifier;
  if (ts.isStringLiteral(moduleSpecifier) && moduleSpecifier.text === typeUtils.INTEROP_SDK_NAME) {
    return false;
  }
  const moduleSymbol = typeChecker.getSymbolAtLocation(moduleSpecifier);
  if (moduleSymbol) {
    const declarations = moduleSymbol.getDeclarations();
    if (declarations?.length) {
      const sourceFile = declarations[0].getSourceFile();
      const importName = getBaseNameFromFilePath(sourceFile.fileName);
      if (importName === typeUtils.INTEROP_SDK_NAME) {
        return false;
      }
    }
  }
  return true;
}

function createESInteropImportDeclaration(context: ts.TransformationContext): ts.ImportDeclaration {
  const esName = context.factory.createIdentifier(typeUtils.INTEROP_NAMESPACE.Dynamic);
  const importClause = context.factory.createImportClause(false, esName, undefined);
  const moduleSpecifier = context.factory.createStringLiteral(typeUtils.INTEROP_SDK_NAME);
  const targetImportStmt = context.factory.createImportDeclaration(undefined, importClause, moduleSpecifier);

  return targetImportStmt;
}
