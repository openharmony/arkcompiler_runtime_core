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
import { InteropCommand, InteropTokens, parseInteropCommand, validateCommandForNode, COMMAND_ALLOWED_KINDS } from './InteropCommandParser';
import { TransformationDiagnostic } from '../TransformationDiagnostic';
import { DiagnosticMessages } from '../TransformationDiagnosticMessages';
import * as typeUtils from '../utils/TypeUtils';

type PrevState = undefined;
type NextState = undefined;
type LocalState = undefined;

export class InteropTagsStage extends TransformationStage<PrevState, NextState, LocalState> {
  constructor(options?: StageOptions) {
    super(
      [InteropTagsTraverser],
      () => undefined,
      () => undefined,
      false,
      options
    );
  }
}

class InteropTagsTraverser extends VisitorTraverser<PrevState, LocalState> {
  constructor(
    context: ts.TransformationContext,
    typeChecker: ts.TypeChecker,
    state: TraverserState<PrevState, LocalState>
  ) {
    super(new DepthFirstAlgorithm(), context, typeChecker, state);
  }

  protected buildVisitors(): Map<ts.SyntaxKind, ts.Visitor[]> {
    const interopTagsVisitor = [ this[FaultID.InteropTags].bind(this) ];
    const allKinds: Set<ts.SyntaxKind> = new Set();
    Object.entries(COMMAND_ALLOWED_KINDS).forEach(([, allowedKinds]) => {
      for (const kind of allowedKinds) {
        allKinds.add(kind);
      }
    });
    const visitors = new Map<ts.SyntaxKind, ts.Visitor[]>();
    allKinds.forEach((kind) => {
      visitors.set(kind, interopTagsVisitor);
    });
    return visitors;
  }

  private [FaultID.InteropTags](node: ts.Node): ts.VisitResult<ts.Node> {
    const originalNode = ts.getOriginalNode(node);
    const tokenList = this.parseInteropTagTokens(originalNode);

    let current: ts.VisitResult<ts.Node> = node;
    for (const tokens of tokenList) {
      const parseResult = parseInteropCommand(tokens);
      if (!parseResult.ok) {
        this.reportDiagnostics(parseResult.diagnostics, originalNode);
        continue;
      }

      const command = parseResult.command;
      const kindError = validateCommandForNode(command, node.kind, tokens.tagName);
      if (kindError) {
        this.reportDiagnostics([kindError], originalNode);
        continue;
      }

      // Apply command to the latest node result; stop if node was removed.
      const next = this.dispatchCommand(command, current ?? node, originalNode);
      current = next;
      if (current === undefined) {
        break;
      }
    }

    return current;
  }

  private dispatchCommand(
    command: InteropCommand,
    node: ts.VisitResult<ts.Node>,
    originalNode: ts.Node
  ): ts.VisitResult<ts.Node> {
    const n = node as ts.Node;
    switch (command.type) {
      case 'noninterop':
        return this.handleNoninterop();
      case 'interop':
        switch (command.subcommand) {
          case 'any':
            return this.handleAny(n);
          case 'break-extends':
            return this.handleBreakExtends(n);
          case 'break-implements':
            return this.handleBreakImplements(n);
          case 'ret':
            return this.handleRet(n, command.targetType, originalNode);
          case 'param':
            return this.handleParam(n, command.index, command.targetType, originalNode);
        }
    }
  }

  /** @noninterop — remove the declaration entirely. */
  private handleNoninterop(): ts.VisitResult<ts.Node> {
    return undefined;
  }

  /** @interop any — replace the declaration with `type Name = Any;`, keeping inheritable modifiers. */
  private handleAny(node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isClassDeclaration(node) && !ts.isInterfaceDeclaration(node) && !ts.isEnumDeclaration(node)) {
      return node;
    }
    const name = node.name;
    if (!name) {
      return node;
    }
    const modifiers = filterTypeAliasInheritableModifiers(node.modifiers);
    return this.context.factory.createTypeAliasDeclaration(
      modifiers,
      name,
      undefined,
      typeUtils.createJSValueTypeNode(this.context)
    );
  }

  /** @interop break-extends — strip the extends heritage clause. */
  private handleBreakExtends(node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isClassDeclaration(node)) {
      const clauses = node.heritageClauses?.filter((c) => c.token !== ts.SyntaxKind.ExtendsKeyword);
      return this.context.factory.updateClassDeclaration(node, node.modifiers, node.name, node.typeParameters, clauses, node.members);
    }
    if (ts.isInterfaceDeclaration(node)) {
      const clauses = node.heritageClauses?.filter((c) => c.token !== ts.SyntaxKind.ExtendsKeyword);
      return this.context.factory.updateInterfaceDeclaration(node, node.modifiers, node.name, node.typeParameters, clauses, node.members);
    }
    return node;
  }

  /** @interop break-implements — strip the implements heritage clause. */
  private handleBreakImplements(node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isClassDeclaration(node)) {
      const clauses = node.heritageClauses?.filter((c) => c.token !== ts.SyntaxKind.ImplementsKeyword);
      return this.context.factory.updateClassDeclaration(node, node.modifiers, node.name, node.typeParameters, clauses, node.members);
    }
    return node;
  }

  /** @interop ret <target-type> - change the return type of a function/method. */
  private handleRet(node: ts.Node, targetType: string, originalNode: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isFunctionDeclaration(node) && !ts.isMethodDeclaration(node) && !ts.isMethodSignature(node)) {
      return node;
    }
    const newReturnType = this.getTypeFromString(targetType);
    if (!newReturnType) {
      this.reportDiagnostics([DiagnosticMessages.interopTagInvalidTargetType(targetType)], originalNode);
      return node;
    }
    if (ts.isFunctionDeclaration(node)) {
      return this.context.factory.updateFunctionDeclaration(
        node,
        node.modifiers,
        node.asteriskToken,
        node.name,
        node.typeParameters,
        node.parameters,
        newReturnType,
        node.body
      );
    }
    if (ts.isMethodDeclaration(node)) {
      return this.context.factory.updateMethodDeclaration(
        node,
        node.modifiers,
        node.asteriskToken,
        node.name,
        node.questionToken,
        node.typeParameters,
        node.parameters,
        newReturnType,
        node.body
      );
    }
    return this.context.factory.updateMethodSignature(
      node,
      node.modifiers,
      node.name,
      node.questionToken,
      node.typeParameters,
      node.parameters,
      newReturnType
    );
  }

  /** @interop param <index> <target-type> - change the type of a function/method/constructor parameter. */
  private handleParam(
    node: ts.Node,
    index: number,
    targetType: string,
    originalNode: ts.Node
  ): ts.VisitResult<ts.Node> {
    if (
      !ts.isFunctionDeclaration(node) &&
      !ts.isConstructorDeclaration(node) &&
      !ts.isMethodDeclaration(node) &&
      !ts.isMethodSignature(node)
    ) {
      return node;
    }
    if (index < 0 || index >= node.parameters.length) {
      return node;
    }
    const newType = this.getTypeFromString(targetType);
    if (!newType) {
      this.reportDiagnostics([DiagnosticMessages.interopTagInvalidTargetType(targetType)], originalNode);
      return node;
    }
    const newParameters = this.replaceParameterType(node.parameters, index, newType);
    return this.updateNodeWithParameters(node, newParameters);
  }

  private updateNodeWithParameters(
    node: ts.FunctionDeclaration | ts.ConstructorDeclaration | ts.MethodDeclaration | ts.MethodSignature,
    newParameters: ts.NodeArray<ts.ParameterDeclaration>
  ): ts.VisitResult<ts.Node> {
    if (ts.isFunctionDeclaration(node)) {
      return this.context.factory.updateFunctionDeclaration(
        node,
        node.modifiers,
        node.asteriskToken,
        node.name,
        node.typeParameters,
        newParameters,
        node.type,
        node.body
      );
    }
    if (ts.isConstructorDeclaration(node)) {
      return this.context.factory.updateConstructorDeclaration(
        node,
        node.modifiers,
        newParameters,
        node.body
      );
    }
    if (ts.isMethodDeclaration(node)) {
      return this.context.factory.updateMethodDeclaration(
        node,
        node.modifiers,
        node.asteriskToken,
        node.name,
        node.questionToken,
        node.typeParameters,
        newParameters,
        node.type,
        node.body
      );
    }
    return this.context.factory.updateMethodSignature(
      node,
      node.modifiers,
      node.name,
      node.questionToken,
      node.typeParameters,
      newParameters,
      node.type
    );
  }

  private replaceParameterType(
    parameters: ts.NodeArray<ts.ParameterDeclaration>,
    index: number,
    newType: ts.TypeNode
  ): ts.NodeArray<ts.ParameterDeclaration> {
    const param = parameters[index];
    const newParam = this.context.factory.updateParameterDeclaration(
      param,
      param.modifiers,
      param.dotDotDotToken,
      param.name,
      param.questionToken,
      newType,
      param.initializer
    );
    const arr = [...parameters];
    arr[index] = newParam;
    return ts.factory.createNodeArray(arr);
  }

  private reportDiagnostics(
    partials: Array<Omit<TransformationDiagnostic, 'range' | 'stage'>>,
    node: ts.Node
  ): void {
    const sourceFile = node.getSourceFile();
    const range = sourceFile
      ? { fileName: sourceFile.fileName, start: node.getStart(sourceFile), length: node.getWidth(sourceFile) }
      : undefined;
    const stage = this.state.stageContext.stageStack.top() ?? '';
    for (const partial of partials) {
      this.state.stageContext.diagnostics.push({ ...partial, stage, range });
    }
  }

  private parseInteropTagTokens(node: ts.Node): InteropTokens[] {
    const tags = ts.getJSDocTags(node);
    const result: InteropTokens[] = [];
    for (const tag of tags) {
      const tagName = tag.tagName.text;
      if (tagName !== 'interop' && tagName !== 'noninterop') {
        continue;
      }
      if (!tag.comment) {
        result.push({ tagName, tokens: [] });
      }
      if (typeof tag.comment !== 'string') {
        continue;
      }
      const tokens = tag.comment.split(/\s+/).filter((token) => token.length > 0);
      result.push({
        tagName,
        tokens
      });
    }
    return result;
  }

  getTypeFromString(typeStr: string): ts.TypeNode | undefined {
    const sourceText = `type _T = ${typeStr};`;
    const sourceFile = ts.createSourceFile('_.ts', sourceText, ts.ScriptTarget.Latest, /*setParentNodes*/ true);
    // parseDiagnostics is an internal property populated during parsing.
    const parseDiagnostics: readonly ts.Diagnostic[] =
      (sourceFile as unknown as { parseDiagnostics?: readonly ts.Diagnostic[] }).parseDiagnostics ?? [];
    if (parseDiagnostics.length > 0) {
      return undefined;
    }
    const stmt = sourceFile.statements[0];
    if (!ts.isTypeAliasDeclaration(stmt)) {
      return undefined;
    }
    // Clear positions on all nodes so TypeScript treats them as synthesized.
    // Without this, the printer uses pos/end to read verbatim text from the
    // *real* source file at those offsets, producing garbage output.
    clearTextRanges(stmt.type);
    return stmt.type;
  }
}

/** Modifier kinds that are valid on 、 type alias declaration and should be preserved. */
const TYPE_ALIAS_INHERITABLE_MODIFIER_KINDS: ReadonlySet<ts.SyntaxKind> = new Set([
  ts.SyntaxKind.ExportKeyword,
  ts.SyntaxKind.DefaultKeyword,
  ts.SyntaxKind.DeclareKeyword
]);

function filterTypeAliasInheritableModifiers(
  modifiers: ts.NodeArray<ts.ModifierLike> | undefined
): readonly ts.Modifier[] | undefined {
  if (!modifiers) {
    return undefined;
  }
  const filtered = modifiers.filter((m): m is ts.Modifier => TYPE_ALIAS_INHERITABLE_MODIFIER_KINDS.has(m.kind));
  return filtered.length > 0 ? filtered : undefined;
}

/**
 * Recursively mark all nodes in a subtree as synthesized (pos = end = -1).
 * TypeScript's printer checks pos < 0 to decide between verbatim and structural
 * emit; nodes parsed from a foreign source file must be cleared before being
 * spliced into a transformation of a different file.
 */
function clearTextRanges(node: ts.Node): void {
  ts.setTextRange(node, { pos: -1, end: -1 });
  ts.forEachChild(node, (child) => {
    clearTextRanges(child);
  });
}
