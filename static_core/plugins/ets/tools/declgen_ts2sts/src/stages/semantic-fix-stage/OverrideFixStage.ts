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

type PrevState = undefined;
type NextState = undefined;
type LocalState = undefined;

export class OverrideFixStage extends TransformationStage<PrevState, NextState, LocalState> {
  constructor(options?: StageOptions) {
    super(
      [OverrideFixTraverser],
      () => undefined,
      () => undefined,
      false,
      options
    );
  }
}

class OverrideFixTraverser extends VisitorTraverser<PrevState, LocalState> {
  constructor(
    context: ts.TransformationContext,
    typeChecker: ts.TypeChecker,
    state: TraverserState<PrevState, LocalState>
  ) {
    super(new DepthFirstAlgorithm(), context, typeChecker, state);
  }

  protected buildVisitors(): Map<ts.SyntaxKind, ts.Visitor[]> {
    return new Map<ts.SyntaxKind, ts.Visitor[]>([
      [ts.SyntaxKind.ClassDeclaration, [this[FaultID.FixAbstractMethodOverride].bind(this)]]
    ]);
  }

  /**
   * Rule: `arkts-fix-abstract-method-override`
   */
  private [FaultID.FixAbstractMethodOverride](node: ts.Node): ts.VisitResult<ts.Node> {
    /*
     * If a class extends an abstract class, ensure every abstract method/property
     * from the parent is overridden in the subclass.  For each abstract member
     * that is missing or has a different signature, a concrete declaration with
     * the same signature is added.
     */
    if (!ts.isClassDeclaration(node)) {
      return node;
    }

    const missingMembers = getMissingAbstractMembers(node, this.typeChecker, this.context);

    return this.context.factory.updateClassDeclaration(
      node,
      node.modifiers,
      node.name,
      node.typeParameters,
      node.heritageClauses,
      this.context.factory.createNodeArray([...node.members, ...missingMembers])
    );
  }
}

function getParentSymbol(node: ts.ClassDeclaration, checker: ts.TypeChecker): ts.Symbol | undefined {
  const extendsClause = node.heritageClauses?.find((hc) => hc.token === ts.SyntaxKind.ExtendsKeyword);
  if (!extendsClause || extendsClause.types.length === 0) {
    return undefined;
  }

  const parentTypeExpr = extendsClause.types[0];
  const parentType = checker.getTypeAtLocation(parentTypeExpr.expression);
  const parentSymbol = parentType.getSymbol();
  return parentSymbol;
}

// Collect abstract methods from the parent.
function getParentAbstractMethods(node: ts.ClassDeclaration, checker: ts.TypeChecker): ts.MethodDeclaration[] {
  const parentSymbol = getParentSymbol(node, checker);
  if (!parentSymbol) {
    return [];
  }

  const parentDecls = parentSymbol.getDeclarations() ?? [];
  const parentClassDecl = parentDecls.find(
    (decl): decl is ts.ClassDeclaration =>
      ts.isClassDeclaration(decl) && (decl.modifiers?.some((m) => m.kind === ts.SyntaxKind.AbstractKeyword) ?? false)
  );
  if (!parentClassDecl) {
    return [];
  }

  const abstractMethods = parentClassDecl.members.filter(
    (member): member is ts.MethodDeclaration =>
      ts.isMethodDeclaration(member) &&
      (member.modifiers?.some((m) => m.kind === ts.SyntaxKind.AbstractKeyword) ?? false)
  );
  return abstractMethods;
}

function compareParametersTypeByChecker(
  params1: ts.NodeArray<ts.ParameterDeclaration>,
  params2: ts.NodeArray<ts.ParameterDeclaration>,
  checker: ts.TypeChecker
): boolean {
  if (params1.length !== params2.length) {
    return false;
  }
  for (let i = 0; i < params1.length; i++) {
    const paramType1 = params1[i].type;
    const paramType2 = params2[i].type;
    const type1 = paramType1 ? checker.getTypeAtLocation(paramType1) : undefined;
    const type2 = paramType2 ? checker.getTypeAtLocation(paramType2) : undefined;
    if (type1 && type2) {
      if (checker.typeToString(type1) !== checker.typeToString(type2)) {
        return false;
      }
    } else if (!!type1 !== !!type2) {
      return false;
    }
  }
  return true;
}

function compareTypeParameters(
  params1: ts.NodeArray<ts.TypeParameterDeclaration> | undefined,
  params2: ts.NodeArray<ts.TypeParameterDeclaration> | undefined
): boolean {
  if (!params1 && !params2) {
    return true;
  }
  if (!params1 || !params2) {
    return false;
  }
  if (params1.length !== params2.length) {
    return false;
  }

  for (let i = 0; i < params1.length; i++) {
    const p1 = params1[i];
    const p2 = params2[i];

    if (p1.name.getText() !== p2.name.getText()) {
      return false;
    }
  }

  return true;
}

function isSameMethodDeclaration(
  member1: ts.MethodDeclaration,
  member2: ts.MethodDeclaration,
  checker: ts.TypeChecker
): boolean {
  if (!compareParametersTypeByChecker(member1.parameters, member2.parameters, checker)) {
    return false;
  }
  if (member1.type && member2.type) {
    const type1 = checker.getTypeAtLocation(member1.type);
    const type2 = checker.getTypeAtLocation(member2.type);
    return checker.typeToString(type1) === checker.typeToString(type2);
  } else if (!!member1.type !== !!member2.type) {
    return false;
  }
  if (!compareTypeParameters(member1.typeParameters, member2.typeParameters)) {
    return false;
  }
  return true;
}

function getMissingAbstractMembers(
  node: ts.ClassDeclaration,
  checker: ts.TypeChecker,
  context: ts.TransformationContext
): ts.ClassElement[] {
  const abstractMethods = getParentAbstractMethods(node, checker);
  if (abstractMethods.length === 0) {
    return [];
  }
  const existingMethods = new Map<string, ts.MethodDeclaration>();
  for (const member of node.members) {
    if (ts.isMethodDeclaration(member) && ts.isIdentifier(member.name)) {
      existingMethods.set(member.name.text, member);
    }
  }

  const missingMembers: ts.ClassElement[] = [];
  for (const abstractMember of abstractMethods) {
    if (!ts.isIdentifier(abstractMember.name)) {
      continue;
    }
    if (existingMethods.has(abstractMember.name.text)) {
      const existingMember = existingMethods.get(abstractMember.name.text)!;
      if (isSameMethodDeclaration(existingMember, abstractMember, checker)) {
        continue;
      }
    }

    // Strip the `abstract` modifier when synthesising the override.
    const newModifiers = [...(abstractMember.modifiers ?? [])].filter(
      (m) => m.kind !== ts.SyntaxKind.AbstractKeyword
    ) as ts.Modifier[];

    missingMembers.push(
      context.factory.createMethodDeclaration(
        newModifiers,
        abstractMember.asteriskToken,
        abstractMember.name,
        abstractMember.questionToken,
        abstractMember.typeParameters,
        abstractMember.parameters,
        abstractMember.type,
        undefined
      )
    );
  }
  return missingMembers;
}
