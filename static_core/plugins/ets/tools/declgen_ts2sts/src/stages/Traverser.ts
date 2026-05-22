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
import { isVisitResultNode, isVisitResultNodeArray, visitVisitResult } from './utils/ASTHelpers';
import { FaultID } from './utils/FaultId';

export interface TraverserState<T, S> {
  publicState: T;
  localState: S;
}

export abstract class Traverser<T, S> {
  public state: TraverserState<T, S>;
  protected context: ts.TransformationContext;
  protected typeChecker: ts.TypeChecker;

  constructor(context: ts.TransformationContext, typeChecker: ts.TypeChecker, state: TraverserState<T, S>) {
    this.context = context;
    this.typeChecker = typeChecker;
    this.state = state;
  }

  abstract traverse(node: ts.SourceFile): ts.SourceFile;
}

export type TraverserConstructor<T, S> = new (
  context: ts.TransformationContext,
  typeChecker: ts.TypeChecker,
  state: TraverserState<T, S>
) => Traverser<T, S>;

/**
 * Strategy interface for traversal algorithms.
 * Implementations define how the AST is walked while delegating node transformation to `visit`.
 */
export interface TraversalAlgorithm {
  traverse(
    node: ts.SourceFile,
    visit: (node: ts.Node) => ts.VisitResult<ts.Node>,
    context: ts.TransformationContext
  ): ts.SourceFile;
}

/**
 * Depth-first (post-order) traversal: children are visited before their parent.
 */
export class DepthFirstAlgorithm implements TraversalAlgorithm {
  traverse(
    node: ts.SourceFile,
    visit: (node: ts.Node) => ts.VisitResult<ts.Node>,
    context: ts.TransformationContext
  ): ts.SourceFile {
    const visitor = (n: ts.Node): ts.VisitResult<ts.Node> => {
      let visitResult: ts.VisitResult<ts.Node> = ts.visitEachChild(n, visitor, context);
      visitResult = visitVisitResult(visitResult, visit);
      return visitResult;
    };
    return visitor(node) as ts.SourceFile;
  }
}

/**
 * Breadth-first (level-order) traversal: all nodes at depth N are visited before depth N+1.
 */
export class BreadthFirstAlgorithm implements TraversalAlgorithm {
  traverse(
    node: ts.SourceFile,
    visit: (node: ts.Node) => ts.VisitResult<ts.Node>,
    context: ts.TransformationContext
  ): ts.SourceFile {
    let current: ts.SourceFile = node;

    for (let targetDepth = 0; ; targetDepth++) {
      let hasNodeOnTargetDepth = false;

      const visitor = (currentNode: ts.Node, depth: number): ts.VisitResult<ts.Node> => {
        let visitResult: ts.VisitResult<ts.Node> = ts.visitEachChild(
          currentNode,
          (child) => visitor(child, depth + 1),
          context
        );

        if (depth === targetDepth) {
          hasNodeOnTargetDepth = true;
          visitResult = visitVisitResult(visitResult, visit);
        }

        return visitResult;
      };

      const next = visitor(current, 0);
      if (!hasNodeOnTargetDepth) {
        break;
      }

      if (!isVisitResultNode(next) || !ts.isSourceFile(next)) {
        return current;
      }

      current = next;
    }

    return current;
  }
}

/**
 * A traverser that dispatches each node to a `SyntaxKind → Visitor[]` map.
 * The traversal order is determined by the injected `TraversalAlgorithm`,
 * allowing any combination of visitor logic and traversal strategy.
 */
export abstract class VisitorTraverser<T, S> extends Traverser<T, S> {
  protected visitors: Map<ts.SyntaxKind, ts.Visitor[]>;
  private readonly algorithm: TraversalAlgorithm;
  private decoratedOnce: boolean;

  constructor(
    algorithm: TraversalAlgorithm,
    context: ts.TransformationContext,
    typeChecker: ts.TypeChecker,
    state: TraverserState<T, S>
  ) {
    super(context, typeChecker, state);
    this.algorithm = algorithm;
    this.decoratedOnce = false;
    this.visitors = this.buildVisitors();
    this.decorateVisitors();
  }

  protected buildVisitors(): Map<ts.SyntaxKind, ts.Visitor[]> {
    return new Map();
  }

  private setOriginNode(result: ts.VisitResult<ts.Node>, origin: ts.Node): void {
    if (isVisitResultNodeArray(result)) {
      result.forEach((resultNode) => {
        ts.setOriginalNode(resultNode, origin);
      });
    } else if (isVisitResultNode(result) && result !== origin) {
      ts.setOriginalNode(result, origin);
    }
  }

  private decorateVisitors(): void {
    if (this.decoratedOnce) {
      return;
    }
    this.decoratedOnce = true;
    this.visitors.forEach((visitorList, kind) => {
      const wrapped = visitorList.map((visitor) => {
        return (node: ts.Node): ts.VisitResult<ts.Node> => {
          const origin = ts.getOriginalNode(node);
          const visitResult = visitor(node);
          this.setOriginNode(visitResult, origin);
          return visitResult;
        };
      });
      this.visitors.set(kind, wrapped);
    });
  }

  visit(node: ts.Node): ts.VisitResult<ts.Node> {
    const autofixes = this.visitors.get(node.kind);
    if (autofixes === undefined) {
      return node;
    }
    let result: ts.VisitResult<ts.Node> = node;
    for (const autofix of autofixes) {
      result = visitVisitResult(result, autofix);
    }
    return result;
  }

  traverse(node: ts.SourceFile): ts.SourceFile {
    return this.algorithm.traverse(node, this.visit.bind(this), this.context);
  }
}
