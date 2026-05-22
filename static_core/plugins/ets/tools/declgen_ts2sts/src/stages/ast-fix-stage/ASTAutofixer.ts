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

import { FaultID } from '../utils/FaultId';
import { VisitorTraverser, TraverserState, DepthFirstAlgorithm } from '../Traverser';
import * as typeUtils from '../utils/TypeUtils';

export class Autofixer extends VisitorTraverser<undefined, undefined> {
  constructor(
    context: ts.TransformationContext,
    typeChecker: ts.TypeChecker,
    state: TraverserState<undefined, undefined>
  ) {
    super(new DepthFirstAlgorithm(), context, typeChecker, state);
  }
  protected buildVisitors(): Map<ts.SyntaxKind, ts.Visitor[]> {
    return new Map<ts.SyntaxKind, ts.Visitor[]>([
      [ts.SyntaxKind.VariableDeclarationList, [this[FaultID.VarDeclaration].bind(this)]],
      [ts.SyntaxKind.PropertyDeclaration, [this[FaultID.PrivateIdentifier].bind(this)]],
      [
        ts.SyntaxKind.SourceFile,
        [
          this[FaultID.ImportAfterStatement].bind(this),
          this[FaultID.LimitExtends].bind(this),
          this[FaultID.AddDeclareToTopLevelInterfaces].bind(this),
          this[FaultID.AddDeclareToTopLevelVariable].bind(this),
          this[FaultID.AddDeclareToTopLevelFunction].bind(this),
          this[FaultID.AddDeclareToTopLevelNamespace].bind(this),
          this[FaultID.AddDeclareToTopLevelClass].bind(this),
          this[FaultID.AddDeclareToTopLevelEnum].bind(this)
        ]
      ],
      [ts.SyntaxKind.ModuleDeclaration, [this[FaultID.Module].bind(this)]],
      [ts.SyntaxKind.ModuleBlock, [this[FaultID.ExportNamespace].bind(this)]],
      [
        ts.SyntaxKind.InterfaceDeclaration,
        [this[FaultID.NoConstructorInInterface].bind(this), this[FaultID.AddDeclareToInterface].bind(this)]
      ],

      [ts.SyntaxKind.FunctionDeclaration, [this[FaultID.DefaultExport].bind(this)]],
      [
        ts.SyntaxKind.ClassDeclaration,
        [this[FaultID.NoPrivateMember].bind(this), this[FaultID.AddDeclareToClass].bind(this)]
      ],
      [ts.SyntaxKind.StructDeclaration, [this[FaultID.StructDeclaration].bind(this)]]
    ]);
  }

  /**
   * Rule: `arkts-no-var`
   */
  private [FaultID.VarDeclaration](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Ensure that all variable declarations are using `let` or `const`.
     */

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

  /**
   * Rule: `arkts-no-module`
   */
  private [FaultID.Module](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Module mapped to namespace in arkts1.2
     */

    if (ts.isModuleDeclaration(node)) {
      const newFlags = node.flags | ts.NodeFlags.Namespace;
      return this.context.factory.createModuleDeclaration(node.modifiers, node.name, node.body, newFlags);
    }

    return node;
  }

  /**
   * Rule: `arkts-no-export-namespace`
   */
  private [FaultID.ExportNamespace](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * In ArkTS 1.2, it is required to explicitly export namespaces in declaration files.
     */

    const statements: ts.Statement[] = [];
    let shouldChange: boolean = true;

    if (ts.isModuleBlock(node)) {
      for (const stmt of node.statements) {
        stmt.forEachChild((child) => {
          if (child.kind === ts.SyntaxKind.ExportKeyword) {
            shouldChange = false;
          }
        });
        if (!shouldChange) {
          // reset initial value
          shouldChange = true;
          statements.push(stmt);
        } else {
          const newDeclaration = transModuleBlockformation(stmt, this.context, stmt.kind);
          statements.push(newDeclaration);
        }
      }
      return this.context.factory.updateModuleBlock(node, [...statements]);
    }
    return node;
  }

  /**
   * Rule: `arkts-no-private-members`
   */
  private [FaultID.NoPrivateMember](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * For members modified by private, they do not appear in the declaration file.
     */
    if (ts.isClassDeclaration(node)) {
      const newMembers = node.members.filter((member) => {
        if (!ts.canHaveModifiers(member)) {
          return true;
        }
        const modifiers = ts.getModifiers(member);
        return !(modifiers === null || modifiers === void 0
          ? void 0
          : modifiers.some((modifier) => {
              return modifier.kind === ts.SyntaxKind.PrivateKeyword;
            }));
      });

      const decorators = ts.getAllDecorators(node) ?? [];
      const modifiers = ts.canHaveModifiers(node) ? (ts.getModifiers(node) ?? []) : [];

      const modifierLike: ts.NodeArray<ts.ModifierLike> | undefined =
        decorators.length || modifiers.length
          ? ts.factory.createNodeArray<ts.ModifierLike>([...decorators, ...modifiers])
          : undefined;

      return this.context.factory.updateClassDeclaration(
        node,
        modifierLike,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        newMembers
      );
    }

    return node;
  }

  /**
   * Rule: `arkts-ESObject-to-object-type`
   */
  private [FaultID.DefaultExport](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isFunctionDeclaration(node) || ts.isClassDeclaration(node)) {
      return exportDefaultAssignment(node, this.context);
    }

    return node;
  }

  /**
   * Rule: `arkts-add-declare-to-classes`
   */
  private [FaultID.AddDeclareToClass](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isClassDeclaration(node)) {
      return node;
    }

    const modifiers = node.modifiers;
    if (!modifiers) {
      return node;
    }

    const isExported = modifiers.some((m) => m.kind === ts.SyntaxKind.ExportKeyword);
    if (!isExported) {
      return node;
    }
    const hasDeclare = modifiers.some((m) => m.kind === ts.SyntaxKind.DeclareKeyword);
    if (hasDeclare) {
      return node;
    }

    const isDefaultExport = modifiers.some((m) => m.kind === ts.SyntaxKind.DefaultKeyword);

    if (isDefaultExport) {
      // For default export class, add declare after 'export default'
      return exportDefaultAssignment(node, this.context);
    } else {
      return node;
    }
  }

  /**
   * Rule: `arkts-add-declare-to-interfaces`
   */
  private [FaultID.AddDeclareToInterface](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isInterfaceDeclaration(node)) {
      return node;
    }

    const modifiers = node.modifiers;
    if (!modifiers) {
      return node;
    }

    const isExported = modifiers.some((m) => m.kind === ts.SyntaxKind.ExportKeyword);
    if (!isExported) {
      return node;
    }
    const hasDeclare = modifiers.some((m) => m.kind === ts.SyntaxKind.DeclareKeyword);
    if (hasDeclare) {
      return node;
    }

    const isDefaultExport = modifiers.some((m) => m.kind === ts.SyntaxKind.DefaultKeyword);

    if (isDefaultExport) {
      // For default export interface, add declare after 'export default'
      return exportDefaultAssignment(node, this.context);
    } else {
      return node;
    }
  }

  /**
   * Rule: `arkts-add-declare-to-exported-variables`
   */
  private [FaultID.AddDeclareToTopLevelVariable](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isSourceFile(node)) {
      return node;
    }

    const statements = node.statements.map((stmt) => {
      if (ts.isVariableStatement(stmt) && stmt.modifiers?.some((m) => m.kind === ts.SyntaxKind.ExportKeyword)) {
        if (stmt.modifiers.some((m) => m.kind === ts.SyntaxKind.DeclareKeyword)) {
          return stmt;
        }

        const newModifiers: ts.Modifier[] = [];
        for (const modifier of stmt.modifiers) {
          newModifiers.push(modifier);
          if (modifier.kind === ts.SyntaxKind.ExportKeyword) {
            newModifiers.push(this.context.factory.createToken(ts.SyntaxKind.DeclareKeyword) as ts.Modifier);
          }
        }

        return this.context.factory.updateVariableStatement(stmt, newModifiers, stmt.declarationList);
      }
      return stmt;
    });

    return this.context.factory.updateSourceFile(node, statements);
  }

  /**
   * Rule: `arkts-add-declare-to-exported-functions`
   */
  private [FaultID.AddDeclareToTopLevelFunction](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isSourceFile(node)) {
      return node;
    }

    const statements = node.statements.map((stmt) => {
      if (ts.isFunctionDeclaration(stmt) && stmt.modifiers?.some((m) => m.kind === ts.SyntaxKind.ExportKeyword)) {
        if (stmt.modifiers.some((m) => m.kind === ts.SyntaxKind.DeclareKeyword)) {
          return stmt;
        }

        if (stmt.modifiers.some((m) => m.kind === ts.SyntaxKind.DefaultKeyword)) {
          return stmt;
        }

        const newModifiers: ts.Modifier[] = [];
        for (const modifier of stmt.modifiers) {
          newModifiers.push(modifier);
          if (modifier.kind === ts.SyntaxKind.ExportKeyword) {
            newModifiers.push(this.context.factory.createToken(ts.SyntaxKind.DeclareKeyword) as ts.Modifier);
          }
        }

        return this.context.factory.updateFunctionDeclaration(
          stmt,
          newModifiers,
          stmt.asteriskToken,
          stmt.name,
          stmt.typeParameters,
          stmt.parameters,
          stmt.type,
          stmt.body
        );
      }
      return stmt;
    });

    return this.context.factory.updateSourceFile(node, statements);
  }

  /**
   * Rule: `arkts-no-constructor-in-interface`
   */
  private [FaultID.NoConstructorInInterface](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Remove constructor method signatures from interface declarations
     */

    if (ts.isInterfaceDeclaration(node)) {
      const updatedMembers: Array<ts.TypeElement> = [];
      for (const member of node.members) {
        if (ts.isMethodSignature(member) && ts.isIdentifier(member.name) && member.name.text === 'constructor') {
          continue;
        }
        updatedMembers.push(member);
      }
      return this.context.factory.updateInterfaceDeclaration(
        node,
        node.modifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        updatedMembers
      );
    }

    return node;
  }

  /*
   * Rule: `arkts-no-limit-extends`
   */
  private [FaultID.LimitExtends](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * If some classes inherit from special classes or interfaces,
     * these classes will be directly removed from the declaration file.
     */

    if (ts.isSourceFile(node)) {
      const importDeclarations: ts.ImportDeclaration[] = [];
      const classDeclarations: ts.ClassDeclaration[] = [];
      const interfaceDeclarations: ts.InterfaceDeclaration[] = [];
      const statements: ts.Statement[] = [];
      for (const stmt of node.statements) {
        if (ts.isImportDeclaration(stmt)) {
          importDeclarations.push(stmt);
        } else if (ts.isInterfaceDeclaration(stmt)) {
          interfaceDeclarations.push(stmt);
        } else if (ts.isClassDeclaration(stmt)) {
          classDeclarations.push(stmt);
        } else {
          statements.push(stmt);
        }
      }

      const importSpecifierNames = getImportSpecifierNames(importDeclarations);

      return this.context.factory.updateSourceFile(node, [
        ...importDeclarations,
        ...statements,
        ...tranClassDeclarationList(classDeclarations, this.context, importSpecifierNames),
        ...tranInterfaceDeclarationList(interfaceDeclarations, this.context, importSpecifierNames)
      ]);
    }

    return node;
  }

  /**
   * Rule: `arkts:add-declare-to-top-level-interfaces`
   */
  private [FaultID.AddDeclareToTopLevelInterfaces](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isSourceFile(node)) {
      return node;
    }

    const statements = node.statements.map((stmt) => {
      if (ts.isInterfaceDeclaration(stmt) && stmt.modifiers?.some((m) => m.kind === ts.SyntaxKind.ExportKeyword)) {
        // If 'declare' already exists, do not add it again
        if (stmt.modifiers.some((m) => m.kind === ts.SyntaxKind.DeclareKeyword)) {
          return stmt;
        }

        // Transform interface nodes by adding the declare keyword
        const newModifiers = [
          ...stmt.modifiers.filter((m) => ts.isModifier(m)),
          this.context.factory.createModifier(ts.SyntaxKind.DeclareKeyword)
        ] as ts.Modifier[];

        return this.context.factory.updateInterfaceDeclaration(
          stmt,
          newModifiers,
          stmt.name,
          stmt.typeParameters,
          stmt.heritageClauses,
          stmt.members
        );
      }
      return stmt;
    });

    return this.context.factory.updateSourceFile(node, statements);
  }

  /**
   * Rule: `keeping the struct in its original form`
   */
  private [FaultID.StructDeclaration](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isStructDeclaration(node)) {
      const filteredMembers = node.members.filter((member) => {
        return !ts.isConstructorDeclaration(member);
      });

      // Create a new struct node, removing only the constructor
      const newStruct = ts.factory.updateStructDeclaration(
        node,
        node.modifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        ts.factory.createNodeArray(filteredMembers)
      );

      return newStruct;
    }

    return node;
  }

  /**
   * Rule: `arkts-add-declare-to-exported-top-level-namespaces`
   */
  private [FaultID.AddDeclareToTopLevelNamespace](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isSourceFile(node)) {
      return node;
    }

    const statements = node.statements.map((stmt) => {
      if (ts.isModuleDeclaration(stmt) && stmt.modifiers?.some((m) => m.kind === ts.SyntaxKind.ExportKeyword)) {
        if (stmt.modifiers.some((m) => m.kind === ts.SyntaxKind.DeclareKeyword)) {
          return stmt;
        }

        const newModifiers: ts.Modifier[] = [];
        for (const modifier of stmt.modifiers) {
          newModifiers.push(modifier);
          if (modifier.kind === ts.SyntaxKind.ExportKeyword) {
            newModifiers.push(this.context.factory.createToken(ts.SyntaxKind.DeclareKeyword) as ts.Modifier);
          }
        }

        return this.context.factory.updateModuleDeclaration(stmt, undefined, newModifiers, stmt.name, stmt.body);
      }
      return stmt;
    });

    return this.context.factory.updateSourceFile(node, statements);
  }

  /**
   * Rule: `arkts-add-declare-to-exported-top-level-classes`
   */
  private [FaultID.AddDeclareToTopLevelClass](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isSourceFile(node)) {
      return node;
    }

    const statements = node.statements.map((stmt) => {
      if (ts.isClassDeclaration(stmt) && stmt.modifiers?.some((m) => m.kind === ts.SyntaxKind.ExportKeyword)) {
        if (stmt.modifiers.some((m) => m.kind === ts.SyntaxKind.DeclareKeyword)) {
          return stmt;
        }

        if (stmt.modifiers.some((m) => m.kind === ts.SyntaxKind.DefaultKeyword)) {
          return stmt;
        }

        const newModifiers: ts.Modifier[] = [];
        for (const modifier of stmt.modifiers) {
          newModifiers.push(modifier as ts.Modifier);
          if (modifier.kind === ts.SyntaxKind.ExportKeyword) {
            newModifiers.push(this.context.factory.createToken(ts.SyntaxKind.DeclareKeyword) as ts.Modifier);
          }
        }

        return this.context.factory.updateClassDeclaration(
          stmt,
          newModifiers,
          stmt.name,
          stmt.typeParameters,
          stmt.heritageClauses,
          stmt.members
        );
      }
      return stmt;
    });

    return this.context.factory.updateSourceFile(node, statements);
  }

  /**
   * Rule: `arkts-add-declare-to-exported-top-level-enums`
   */
  private [FaultID.AddDeclareToTopLevelEnum](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * This rule is to make the behavior same for both ts inputs and dts inputs
     */
    if (!ts.isSourceFile(node)) {
      return node;
    }

    const statements = node.statements.map((stmt) => {
      if (ts.isEnumDeclaration(stmt) && stmt.modifiers?.some((m) => m.kind === ts.SyntaxKind.ExportKeyword)) {
        if (stmt.modifiers.some((m) => m.kind === ts.SyntaxKind.DeclareKeyword)) {
          return stmt;
        }

        if (stmt.modifiers.some((m) => m.kind === ts.SyntaxKind.DefaultKeyword)) {
          return stmt;
        }

        const newModifiers: ts.Modifier[] = [];
        for (const modifier of stmt.modifiers) {
          newModifiers.push(modifier as ts.Modifier);
          if (modifier.kind === ts.SyntaxKind.ExportKeyword) {
            newModifiers.push(this.context.factory.createToken(ts.SyntaxKind.DeclareKeyword) as ts.Modifier);
          }
        }

        return this.context.factory.updateEnumDeclaration(stmt, newModifiers, stmt.name, stmt.members);
      }
      return stmt;
    });

    return this.context.factory.updateSourceFile(node, statements);
  }
}

/**
 * Modify the ModuleBlock node to add Export so that it can be referenced by external methods.
 */
function transModuleBlockformation(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  kindType: ts.SyntaxKind
): ts.Statement {
  const exportToken = context.factory.createToken(ts.SyntaxKind.ExportKeyword);
  return updateNodetoAddExport(stmt, context, kindType, exportToken);
}

/**
 * Modify the node node and add the export keyword.
 */
function updateNodetoAddExport(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  kindType: ts.SyntaxKind,
  exportToken: ts.Modifier
): ts.Statement {
  switch (kindType) {
    case ts.SyntaxKind.VariableDeclaration:
    case ts.SyntaxKind.VariableStatement:
      return updateVariableOrStatement(stmt, context, exportToken);
    case ts.SyntaxKind.ModuleDeclaration:
      return updateModuleDeclaration(stmt, context, exportToken);
    case ts.SyntaxKind.FunctionDeclaration:
      return updateFunctionDeclaration(stmt, context, exportToken);
    case ts.SyntaxKind.InterfaceDeclaration:
      return updateInterfaceDeclaration(stmt, context, exportToken);
    case ts.SyntaxKind.ClassDeclaration:
      return updateClassDeclaration(stmt, context, exportToken);
    case ts.SyntaxKind.TypeAliasDeclaration:
      return updateTypeAliasDeclaration(stmt, context, exportToken);
    case ts.SyntaxKind.EnumDeclaration:
      return updateEnumDeclaration(stmt, context, exportToken);
    default:
      return stmt;
  }
}

function updateVariableOrStatement(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  exportToken: ts.Modifier
): ts.Statement {
  if (ts.isVariableStatement(stmt)) {
    return context.factory.updateVariableStatement(
      stmt,
      [exportToken, ...(stmt.modifiers || [])],
      stmt.declarationList
    );
  }
  throw new Error('Unsupported statement type');
}

function updateModuleDeclaration(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  exportToken: ts.Modifier
): ts.Statement {
  if (ts.isModuleDeclaration(stmt)) {
    return context.factory.updateModuleDeclaration(
      stmt,
      [exportToken, ...(stmt.modifiers || [])],
      stmt.name,
      stmt.body
    );
  }
  return stmt;
}

function updateFunctionDeclaration(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  exportToken: ts.Modifier
): ts.Statement {
  if (ts.isFunctionDeclaration(stmt)) {
    return context.factory.updateFunctionDeclaration(
      stmt,
      [exportToken, ...(stmt.modifiers || [])],
      stmt.asteriskToken,
      stmt.name,
      stmt.typeParameters,
      stmt.parameters,
      stmt.type,
      stmt.body
    );
  }
  return stmt;
}

function updateInterfaceDeclaration(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  exportToken: ts.Modifier
): ts.Statement {
  if (ts.isInterfaceDeclaration(stmt)) {
    return context.factory.updateInterfaceDeclaration(
      stmt,
      [exportToken, ...(stmt.modifiers || [])],
      stmt.name,
      stmt.typeParameters,
      stmt.heritageClauses,
      stmt.members
    );
  }
  return stmt;
}

function updateClassDeclaration(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  exportToken: ts.Modifier
): ts.Statement {
  if (ts.isClassDeclaration(stmt)) {
    return context.factory.updateClassDeclaration(
      stmt,
      [exportToken, ...(stmt.modifiers || [])],
      stmt.name,
      stmt.typeParameters,
      stmt.heritageClauses,
      stmt.members
    );
  }
  return stmt;
}

function updateTypeAliasDeclaration(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  exportToken: ts.Modifier
): ts.Statement {
  if (ts.isTypeAliasDeclaration(stmt)) {
    return context.factory.updateTypeAliasDeclaration(
      stmt,
      [exportToken, ...(stmt.modifiers || [])],
      stmt.name,
      stmt.typeParameters,
      stmt.type
    );
  }
  return stmt;
}

function updateEnumDeclaration(
  stmt: ts.Statement,
  context: ts.TransformationContext,
  exportToken: ts.Modifier
): ts.Statement {
  if (ts.isEnumDeclaration(stmt)) {
    return context.factory.updateEnumDeclaration(
      stmt,
      [exportToken, ...(stmt.modifiers || [])],
      stmt.name,
      stmt.members
    );
  }
  return stmt;
}

function exportDefaultAssignment(
  node: ts.FunctionDeclaration | ts.ClassDeclaration | ts.InterfaceDeclaration,
  context: ts.TransformationContext
): ts.VisitResult<ts.Node> {
  const modifiers = node.modifiers;

  if (modifiers === undefined) {
    return node;
  }

  if (modifiers.some((modifier) => modifier.kind === ts.SyntaxKind.DefaultKeyword)) {
    const newModifiers = [...modifiers];

    if (!modifiers.some((modifier) => modifier.kind === ts.SyntaxKind.DeclareKeyword)) {
      const declareModifier = context.factory.createModifier(ts.SyntaxKind.DeclareKeyword);

      const defaultIndex = modifiers.findIndex((mod) => mod.kind === ts.SyntaxKind.DefaultKeyword);
      if (defaultIndex !== -1) {
        newModifiers.splice(defaultIndex + 1, 0, declareModifier);
      } else {
        newModifiers.push(declareModifier);
      }
    }

    const safeModifiers = newModifiers as ts.Modifier[];

    return updateNodeWithModifiers(node, safeModifiers, context);
  }

  return node;
}

function updateNodeWithModifiers(
  node: ts.FunctionDeclaration | ts.ClassDeclaration | ts.InterfaceDeclaration,
  modifiers: ts.Modifier[],
  context: ts.TransformationContext
): ts.VisitResult<ts.Node> {
  if (ts.isFunctionDeclaration(node)) {
    return updateFunctionDeclarationWithModifiers(node, modifiers, context);
  } else if (ts.isClassDeclaration(node)) {
    return updateClassDeclarationWithModifiers(node, modifiers, context);
  } else if (ts.isInterfaceDeclaration(node)) {
    return updateInterfaceDeclarationWithModifiers(node, modifiers, context);
  }

  return node;
}

function updateFunctionDeclarationWithModifiers(
  node: ts.FunctionDeclaration,
  modifiers: ts.Modifier[],
  context: ts.TransformationContext
): ts.FunctionDeclaration {
  return context.factory.updateFunctionDeclaration(
    node,
    modifiers,
    node.asteriskToken,
    node.name,
    node.typeParameters,
    node.parameters,
    node.type,
    node.body
  );
}

function updateClassDeclarationWithModifiers(
  node: ts.ClassDeclaration,
  modifiers: ts.Modifier[],
  context: ts.TransformationContext
): ts.ClassDeclaration {
  return context.factory.updateClassDeclaration(
    node,
    modifiers,
    node.name,
    node.typeParameters,
    node.heritageClauses,
    node.members
  );
}

function updateInterfaceDeclarationWithModifiers(
  node: ts.InterfaceDeclaration,
  modifiers: ts.Modifier[],
  context: ts.TransformationContext
): ts.InterfaceDeclaration {
  return context.factory.updateInterfaceDeclaration(
    node,
    modifiers,
    node.name,
    node.typeParameters,
    node.heritageClauses,
    node.members
  );
}

function tranDeclarationList<T extends ts.ClassDeclaration | ts.InterfaceDeclaration>(
  declarations: T[],
  context: ts.TransformationContext,
  promises: string[]
): T[] {
  return declarations.map((decl) => {
    const heritageClause = decl.heritageClauses?.find((clause) => clause.token === ts.SyntaxKind.ExtendsKeyword);
    const expressions = expressionList(heritageClause?.types);

    if (shouldCreateTypeAlias(decl, expressions, promises)) {
      return createTypeAliasForClass(decl, context) as T;
    } else {
      return decl;
    }
  });
}

function shouldCreateTypeAlias<T extends ts.ClassDeclaration | ts.InterfaceDeclaration>(
  decl: T,
  expressions: string[],
  promises: string[]
): boolean {
  return (
    expressions.some((className) => typeUtils.FINAL_CLASS.includes(className)) &&
    !promises.some((promise) => expressions.includes(promise))
  );
}

function tranClassDeclarationList(
  classDeclarations: ts.ClassDeclaration[],
  context: ts.TransformationContext,
  promises: string[]
): ts.ClassDeclaration[] {
  return tranDeclarationList(classDeclarations, context, promises);
}

function createTypeAliasForClass(
  decl: ts.ClassDeclaration | ts.InterfaceDeclaration,
  context: ts.TransformationContext
): ts.TypeAliasDeclaration | ts.ClassDeclaration | ts.InterfaceDeclaration {
  if (decl.modifiers !== undefined && decl.name !== undefined) {
    return context.factory.createTypeAliasDeclaration(
      filterModifiers(decl.modifiers as ts.NodeArray<ts.Modifier>),
      decl.name,
      decl.typeParameters,
      context.factory.createTypeReferenceNode(typeUtils.JSVALUE)
    );
  } else {
    return decl;
  }
}

function filterModifiers(modifiers: ts.NodeArray<ts.Modifier>): ts.NodeArray<ts.Modifier> | undefined {
  const filteredModifiers = modifiers.filter(
    (mod) => mod.kind !== undefined && mod.kind !== ts.SyntaxKind.DeclareKeyword
  );
  return filteredModifiers.length > 0
    ? ts.factory.createNodeArray(filteredModifiers, modifiers.hasTrailingComma)
    : undefined;
}

function tranInterfaceDeclarationList(
  interfaceDeclarations: ts.InterfaceDeclaration[],
  context: ts.TransformationContext,
  promises: string[]
): ts.InterfaceDeclaration[] {
  return tranDeclarationList(interfaceDeclarations, context, promises);
}

function expressionList(typeArguments?: readonly ts.ExpressionWithTypeArguments[]): string[] {
  const expressions: string[] = [];
  if (typeArguments !== undefined && typeArguments.length > 0) {
    for (const arg of typeArguments) {
      if (arg.expression !== undefined && ts.isIdentifier(arg.expression)) {
        expressions.push(arg.expression.text);
      }
    }
  }
  return expressions;
}

function isFinalClassImport(element: ts.ImportSpecifier): boolean {
  return typeUtils.FINAL_CLASS.includes(element.name.text);
}

function collectFinalClassImports(namedImports: ts.NamedImports, promises: string[]): void {
  namedImports.elements.forEach((element) => {
    if (ts.isImportSpecifier(element) && isFinalClassImport(element)) {
      promises.push(element.name.text);
    }
  });
}

function getImportSpecifierNames(importSpecifierNodes: ts.ImportDeclaration[]): string[] {
  const promises: string[] = [];

  importSpecifierNodes.forEach((item) => {
    if (ts.isImportDeclaration(item)) {
      const namedImports = item.importClause?.namedBindings as ts.NamedImports | undefined;
      if (namedImports !== undefined && ts.isNamedImports(namedImports)) {
        collectFinalClassImports(namedImports, promises);
      }
    }
  });

  return promises;
}
