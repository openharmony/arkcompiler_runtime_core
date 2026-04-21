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
import { VisitorTraverser, DepthFirstAlgorithm, TraverserState } from '../Traverser';

export type ConversionLocalState = {
  existingTypeNames: Set<string>;
  pendingDefaultImports: Map<string, string>;
  defaultAccessMap: Map<string, string>;
};

type PrevState = undefined;
type LocalState = ConversionLocalState;

export class PreScan extends VisitorTraverser<PrevState, LocalState> {
  constructor(
    context: ts.TransformationContext,
    typeChecker: ts.TypeChecker,
    state: TraverserState<PrevState, LocalState>
  ) {
    super(new DepthFirstAlgorithm(), context, typeChecker, state);
  }

  private addName(name: string | undefined): void {
    if (name) {
      this.state.localState.existingTypeNames.add(name);
    }
  }

  private collectImportNames(clause: ts.ImportClause): void {
    // default import: import Foo from '...'
    this.addName(clause.name?.text);

    const bindings = clause.namedBindings;
    if (!bindings) {
      return;
    }
    // namespace import: import * as ns from '...'
    if (ts.isNamespaceImport(bindings)) {
      this.addName(bindings.name.text);
      return;
    }
    // named imports: import { A, B as C } from '...'
    if (ts.isNamedImports(bindings)) {
      for (const spec of bindings.elements) {
        this.addName(spec.name.text);
      }
    }
  }

  visit(node: ts.Node): ts.VisitResult<ts.Node> {
    // Type declarations
    if (
      ts.isInterfaceDeclaration(node) ||
      ts.isClassDeclaration(node) ||
      ts.isTypeAliasDeclaration(node) ||
      ts.isEnumDeclaration(node) ||
      ts.isModuleDeclaration(node) ||
      ts.isFunctionDeclaration(node)
    ) {
      this.addName(node.name?.text);
      return node;
    }

    if (ts.isVariableDeclaration(node) && ts.isIdentifier(node.name)) {
      this.addName(node.name.text);
      return node;
    }

    if (ts.isImportDeclaration(node) && node.importClause) {
      this.collectImportNames(node.importClause);
    }

    return node;
  }
}
