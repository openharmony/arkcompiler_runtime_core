/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import * as ts from "typescript";

import * as ets from "./ETSHelpers";

/**
 * can't use property .getChildCount because generated nodes have no real position,
 * and Node without a real position cannot be scanned and thus has no token nodes. user has to use
 * forEachChild and collect the result if that's fine
 */
function getChildCount(node: ts.Node): number {
  let cnt: number = 0;

  node.forEachChild((c) => {
    cnt += 1;
  });

  return cnt;
}

/**
 * for the same reasons as above, poperty getChildAt can return undefined
 * if previous transformations had happened
 */
function getChildAt(node: ts.Node, idx: number): ts.Node {
  let cnt: number = 0;

  let res = node.forEachChild((c) => {
    if (cnt === idx) {
      return c;
    }
    cnt++;
    return undefined;
  });

  if (res === undefined) {
    throw new Error(`index ${idx} out of range ${getChildCount(node)}`);
  }

  return res;
}

function entityNameToString(name: ts.EntityName): string {
  if (ts.isIdentifier(name)) {
    return name.escapedText.toString();
  } else {
    return entityNameToString(name.left) + entityNameToString(name.right);
  }
}

function isNull(node: ts.Node): boolean {
  if (node.kind === ts.SyntaxKind.NullKeyword) {
    return true;
  }

  if (!ts.isLiteralTypeNode(node)) {
    return false;
  }

  return (
    getChildCount(node) == 1 &&
    node.getChildAt(0).kind == ts.SyntaxKind.NullKeyword
  );
}

function isReference(node: ts.Node): boolean {
  return (
    !isNull(node) &&
    node.kind != ts.SyntaxKind.BooleanKeyword &&
    node.kind != ts.SyntaxKind.NumberKeyword
  );
}

function createJSValue(ctx: ts.TransformationContext) {
  let jsvalue = ctx.factory.createTypeReferenceNode(ets.ETSType.JSVALUE);

  return jsvalue;
}

function visitKeywordType(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isToken(node)) {
    return undefined;
  }

  switch (node.kind) {
    case ts.SyntaxKind.AnyKeyword:
    case ts.SyntaxKind.ObjectKeyword:
    case ts.SyntaxKind.SymbolKeyword:
    case ts.SyntaxKind.FunctionKeyword:
    case ts.SyntaxKind.UnknownKeyword:
    case ts.SyntaxKind.UndefinedKeyword:
      return createJSValue(ctx);
    default:
      return undefined;
  }
}

function visitTypeLiteral(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (ts.isTypeLiteralNode(node)) {
    return createJSValue(ctx);
  }

  return undefined;
}

function visitTuple(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (ts.isTupleTypeNode(node)) {
    return ctx.factory.createArrayTypeNode(createJSValue(ctx));
  }

  return undefined;
}

function visitIntersectionType(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (ts.isIntersectionTypeNode(node)) {
    return createJSValue(ctx);
  }

  return undefined;
}

function visitPrivateProperty(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isPrivateIdentifier(node)) {
    return undefined;
  }

  return ctx.factory.createIdentifier(`// private`);
}

function visitTypeReference(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  const TS2ETS = new Map([
    // unsupported types
    ["Object", ets.ETSType.JSVALUE],
    ["Symbol", ets.ETSType.JSVALUE],
    ["Function", ets.ETSType.JSVALUE],
    ["AsyncFunction", ets.ETSType.JSVALUE],
    ["AsyncGenerator", ets.ETSType.JSVALUE],
    ["AsyncGeneratorFunction", ets.ETSType.JSVALUE],
    ["AsyncIterator", ets.ETSType.JSVALUE],
    ["Generator", ets.ETSType.JSVALUE],
    ["GeneratorFunction", ets.ETSType.JSVALUE],
    ["Iterator", ets.ETSType.JSVALUE],
    ["Proxy", ets.ETSType.JSVALUE],
    ["Reflect", ets.ETSType.JSVALUE],
    ["TypedArray", ets.ETSType.JSVALUE],

    // utility types
    ["Awaited", ets.ETSType.JSVALUE],
    ["Partial", ets.ETSType.JSVALUE],
    ["Required", ets.ETSType.JSVALUE],
    ["Readonly", ets.ETSType.JSVALUE],
    ["Record", ets.ETSType.JSVALUE],
    ["Pick", ets.ETSType.JSVALUE],
    ["Omit", ets.ETSType.JSVALUE],
    ["Exclude", ets.ETSType.JSVALUE],
    ["Extract", ets.ETSType.JSVALUE],
    ["NonNullable", ets.ETSType.JSVALUE],
    ["Parameters", ets.ETSType.JSVALUE],
    ["ConstructorParameters", ets.ETSType.JSVALUE],
    ["ReturnType", ets.ETSType.JSVALUE],
    ["InstanceType", ets.ETSType.JSVALUE],
    ["ThisParameterType", ets.ETSType.JSVALUE],
    ["OmitThisParameter", ets.ETSType.JSVALUE],
    ["ThisType", ets.ETSType.JSVALUE],

    // supported types
    ["EvalError", ets.ETSType.ERROR],
  ]);

  if (!ts.isTypeReferenceNode(node)) {
    return undefined;
  }

  let etsType = TS2ETS.get(entityNameToString(node.typeName));
  if (etsType === undefined) {
    return undefined;
  }

  return ctx.factory.createTypeReferenceNode(etsType, undefined);
}

function visitParameter(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isParameter(node)) {
    return undefined;
  }

  let param = node as ts.ParameterDeclaration;

  if (param.questionToken === undefined) {
    return undefined;
  }

  if (param.type === undefined || isReference(param.type)) {
    return undefined;
  }

  let type = ctx.factory.createUnionTypeNode([
    param.type,
    ctx.factory.createLiteralTypeNode(ctx.factory.createNull()),
  ]);

  return ctx.factory.createParameterDeclaration(
    node.modifiers,
    node.dotDotDotToken,
    node.name,
    undefined,
    type,
    undefined
  );
}

function visitPropertySignature(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isPropertySignature(node)) {
    return undefined;
  }

  let prop = node as ts.PropertySignature;

  if (prop.questionToken == undefined) {
    return undefined;
  }

  if (prop.type === undefined || isReference(prop.type)) {
    return undefined;
  }

  let type = ctx.factory.createUnionTypeNode([
    prop.type,
    ctx.factory.createLiteralTypeNode(ctx.factory.createNull()),
  ]);

  return ctx.factory.createPropertySignature(
    node.modifiers,
    node.name,
    undefined,
    type
  );
}

function visitPropertyDeclaration(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isPropertyDeclaration(node)) {
    return undefined;
  }

  let prop = node as ts.PropertyDeclaration;

  if (prop.questionToken == undefined) {
    return undefined;
  }

  if (prop.type === undefined || isReference(prop.type)) {
    return undefined;
  }

  let type = ctx.factory.createUnionTypeNode([
    prop.type,
    ctx.factory.createLiteralTypeNode(ctx.factory.createNull()),
  ]);

  return ctx.factory.createPropertyDeclaration(
    node.modifiers,
    node.name,
    undefined,
    type,
    node.initializer
  );
}

function visitThisReturnType(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isThisTypeNode(node)) {
    return undefined;
  }

  if (ts.isMethodDeclaration(node.parent)) {
    const classDecl = node.parent.parent;
    if (ts.isClassDeclaration(classDecl)) {
      return classDecl.name
        ? ctx.factory.createTypeReferenceNode(classDecl.name)
        : createJSValue(ctx);
    }
  }

  if (ts.isMethodSignature(node.parent)) {
    const ifaceDecl = node.parent.parent;
    if (ts.isInterfaceDeclaration(ifaceDecl)) {
      return ctx.factory.createTypeReferenceNode(ifaceDecl.name);
    }
  }

  return undefined;
}

function visitConditionalType(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (ts.isConditionalTypeNode(node)) {
    return createJSValue(ctx);
  }

  return undefined;
}

function visitIndexedAccessType(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isIndexedAccessTypeNode(node)) {
    return undefined;
  }

  let res = checker.typeToTypeNode(
    checker.getTypeAtLocation(node),
    undefined,
    undefined
  );

  return res ? visitNode(res, ctx, checker) : undefined;
}

function visitTypeQuery(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isTypeQueryNode(node)) {
    return undefined;
  }

  let res = checker.typeToTypeNode(
    checker.getTypeAtLocation(node),
    undefined,
    undefined
  );

  return res ? visitNode(res, ctx, checker) : undefined;
}

function visitCtorSignature(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isInterfaceDeclaration(node)) {
    return undefined;
  }

  let alias = undefined;
  node.forEachChild((c) => {
    if (!ts.isConstructSignatureDeclaration(c)) {
      return;
    }

    alias = ctx.factory.createTypeAliasDeclaration(
      node.modifiers,
      node.name,
      node.typeParameters,
      createJSValue(ctx)
    );
  });

  return alias;
}

function visitMappedType(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (ts.isMappedTypeNode(node)) {
    return createJSValue(ctx);
  }

  return undefined;
}

function visitKeyofTypeOperator(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isTypeOperatorNode(node)) {
    return undefined;
  }

  if (node.operator !== ts.SyntaxKind.KeyOfKeyword) {
    return undefined;
  }

  return createJSValue(ctx);
}

function visitCallSignature(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isInterfaceDeclaration(node)) {
    return undefined;
  }

  let alias = undefined;
  node.forEachChild((c) => {
    if (!ts.isCallSignatureDeclaration(c)) {
      return;
    }

    alias = ctx.factory.createTypeAliasDeclaration(
      node.modifiers,
      node.name,
      node.typeParameters,
      createJSValue(ctx)
    );
  });

  return alias;
}

function visitIndexedSignature(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isClassDeclaration(node) && !ts.isInterfaceDeclaration(node)) {
    return undefined;
  }

  const objName = node.name;
  if (objName === undefined) {
    return undefined;
  }

  let alias = undefined;
  node.forEachChild((c) => {
    if (!ts.isIndexSignatureDeclaration(c)) {
      return;
    }

    alias = ctx.factory.createTypeAliasDeclaration(
      node.modifiers,
      objName,
      node.typeParameters,
      createJSValue(ctx)
    );
  });

  return alias;
}

function visitCtorType(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isConstructorTypeNode(node)) {
    return undefined;
  }

  return createJSValue(ctx);
}

function visitReadonlyParam(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isParameter(node)) {
    return undefined;
  }

  if (node.type == undefined) {
    return undefined;
  }

  if (!ts.isTypeOperatorNode(node.type)) {
    return undefined;
  }

  return ctx.factory.createParameterDeclaration(
    node.modifiers,
    node.dotDotDotToken,
    node.name,
    node.questionToken,
    node.type.type,
    node.initializer
  );
}

function visitImportEquals(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isImportEqualsDeclaration(node)) {
    return undefined;
  }

  if (ts.isEntityName(node.moduleReference)) {
    return undefined;
  }

  if (!ts.isStringLiteral(node.moduleReference.expression)) {
    return undefined;
  }

  return ctx.factory.createImportDeclaration(
    undefined,
    ctx.factory.createImportClause(
      false,
      undefined,
      ctx.factory.createNamespaceImport(node.name)
    ),
    ctx.factory.createStringLiteral(node.moduleReference.expression.text),
    undefined
  );
}

function visitImportDefaultAs(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isSourceFile(node) && !ts.isModuleBlock(node)) {
    return undefined;
  }

  const updatedStmts: ts.Statement[] = [];

  node.statements.forEach((s) => {
    if (!ts.isImportDeclaration(s)) {
      updatedStmts.push(s);
      return;
    }

    const importClause = s.importClause;
    if (!importClause) {
      updatedStmts.push(s);
      return;
    }

    const namedImports = importClause.namedBindings;
    if (!namedImports || !ts.isNamedImports(namedImports)) {
      updatedStmts.push(s);
      return;
    }

    let nameSpecifiers: Array<ts.ImportSpecifier> = [];
    let defaultAsSpecifiers: Array<ts.ImportSpecifier> = [];

    namedImports.elements.forEach((e) => {
      if (e.propertyName?.escapedText === "default") {
        defaultAsSpecifiers.push(e);
      } else {
        nameSpecifiers.push(e);
      }
    });

    if (nameSpecifiers.length !== 0) {
      const clause = ctx.factory.createImportClause(
        importClause.isTypeOnly,
        importClause.name,
        ctx.factory.createNamedImports(nameSpecifiers)
      );

      updatedStmts.push(
        ctx.factory.createImportDeclaration(
          s.modifiers,
          clause,
          s.moduleSpecifier,
          undefined
        )
      );
    }

    defaultAsSpecifiers.forEach((d) => {
      const clause = ctx.factory.createImportClause(
        importClause.isTypeOnly,
        d.name,
        undefined
      );

      updatedStmts.push(
        ctx.factory.createImportDeclaration(
          s.modifiers,
          clause,
          s.moduleSpecifier,
          undefined
        )
      );
    });
  });

  if (ts.isSourceFile(node)) {
    return ctx.factory.updateSourceFile(node, updatedStmts);
  }

  if (ts.isModuleBlock(node)) {
    return ctx.factory.createModuleBlock(updatedStmts);
  }

  return undefined;
}

function visitJSExetnsion(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isImportDeclaration(node)) {
    return undefined;
  }

  if (!ts.isStringLiteral(node.moduleSpecifier)) {
    throw new Error(
      `grammar error in import ${node.getText()}. module specifier is not a string literal`
    );
  }

  const jsExt = ".js";
  const moduleSpecifier = node.moduleSpecifier.text;

  if (moduleSpecifier.endsWith(jsExt)) {
    return ctx.factory.createImportDeclaration(
      node.modifiers,
      node.importClause,
      ctx.factory.createStringLiteral(moduleSpecifier.replace(jsExt, "")),
      undefined
    );
  }

  return undefined;
}

function visitImportAssertion(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isImportDeclaration(node)) {
    return undefined;
  }

  if (node.assertClause === undefined) {
    return undefined;
  }

  return ctx.factory.createImportDeclaration(
    node.modifiers,
    node.importClause,
    node.moduleSpecifier,
    undefined
  );
}

function visitSpecialImport(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node | undefined {
  if (!ts.isImportDeclaration(node)) {
    return undefined;
  }

  const importClause = node.importClause;
  if (importClause === undefined) {
    return undefined;
  }

  if (importClause.isTypeOnly === false) {
    return undefined;
  }

  return ctx.factory.createImportDeclaration(
    node.modifiers,
    ctx.factory.createImportClause(
      false,
      importClause.name,
      importClause.namedBindings
    ),
    node.moduleSpecifier,
    undefined
  );
}

function visitNode(
  node: ts.Node,
  ctx: ts.TransformationContext,
  checker: ts.TypeChecker
): ts.Node {
  let visitors = [
    visitKeywordType,
    visitTypeLiteral,
    visitTuple,
    visitIntersectionType,
    visitTypeReference,
    visitPrivateProperty,
    visitParameter,
    visitPropertySignature,
    visitPropertyDeclaration,
    visitThisReturnType,
    visitConditionalType,
    visitIndexedAccessType,
    visitTypeQuery,
    visitCtorSignature,
    visitMappedType,
    visitKeyofTypeOperator,
    visitCallSignature,
    visitCtorType,
    visitReadonlyParam,
    visitImportEquals,
    visitImportDefaultAs,
    visitJSExetnsion,
    visitImportAssertion,
    visitSpecialImport,
    visitIndexedSignature,
  ];

  for (const visitor of visitors) {
    let visitResult = visitor(node, ctx, checker);
    if (visitResult !== undefined) {
      return visitResult;
    }
  }

  return node;
}

export function transformAST(
  checker: ts.TypeChecker,
  ctx: ts.TransformationContext
): ts.Transformer<ts.SourceFile | ts.Bundle> {
  return (rootNode) => {
    function transformNode(node: ts.Node): ts.Node {
      node = ts.visitEachChild(node, transformNode, ctx);
      return visitNode(node, ctx, checker);
    }

    return ts.visitNode(rootNode, transformNode) as ts.SourceFile | ts.Bundle;
  };
}
