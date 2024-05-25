/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

import { FaultID } from '../utils/lib/FaultId';
import { visitVisitResult } from './utils/ASTHelpers';

export class Autofixer {
  private readonly typeChecker: ts.TypeChecker;
  private readonly context: ts.TransformationContext;

  constructor(typeChecker: ts.TypeChecker, context: ts.TransformationContext) {
    this.typeChecker = typeChecker;
    this.context = context;
  }

  private readonly autofixes = new Map<ts.SyntaxKind, ts.Visitor[]>([
    [ts.SyntaxKind.VariableDeclarationList, [this[FaultID.VarDeclaration].bind(this)]],
    [ts.SyntaxKind.PropertyDeclaration, [this[FaultID.PrivateIdentifier].bind(this)]],
    [ts.SyntaxKind.SourceFile, [this[FaultID.ImportAfterStatement].bind(this)]]
  ]);

  fixNode(node: ts.Node): ts.VisitResult<ts.Node> {
    const autofixes = this.autofixes.get(node.kind);

    if (autofixes === undefined) {
      return node;
    }

    let result: ts.VisitResult<ts.Node> = node;

    for (const autofix of autofixes) {
      result = visitVisitResult(result, autofix);
    }

    return result;
  }

  /**
   * Rule: `arkts-no-var`
   */
  private [FaultID.VarDeclaration](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isVariableDeclarationList(node)) {
      const isLetDeclaration = node.flags & ts.NodeFlags.Let;
      const isConstDeclaration = node.flags & ts.NodeFlags.Const;

      if (!isLetDeclaration && !isConstDeclaration) {
        const newFlags = node.flags | ts.NodeFlags.Let;
        return this.context.factory.createVariableDeclarationList(node.declarations, newFlags);
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-private-identifiers`
   */
  private [FaultID.PrivateIdentifier](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /*
     * Since we can access only public members of the imported dynamic class,
     * there is no need to fix private fields, we just do not emit them
     */
    if (ts.isPropertyDeclaration(node)) {
      if (
        ts.isPrivateIdentifier(node.name) ||
        node.modifiers?.find((v) => {
          return v.kind === ts.SyntaxKind.PrivateKeyword;
        })
      ) {
        return undefined;
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-misplaced-imports`
   */
  private [FaultID.ImportAfterStatement](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * This algorithm is very very bad for both memory and performance.
     * Redo later, when implementing other autofixes
     */

    if (ts.isSourceFile(node)) {
      const importDeclarations: ts.Statement[] = [];
      const statements: ts.Statement[] = [];

      for (const stmt of node.statements) {
        if (ts.isImportDeclaration(stmt)) {
          importDeclarations.push(stmt);
        } else {
          statements.push(stmt);
        }
      }

      return this.context.factory.updateSourceFile(node, [...importDeclarations, ...statements]);
    }

    return node;
  }
}
