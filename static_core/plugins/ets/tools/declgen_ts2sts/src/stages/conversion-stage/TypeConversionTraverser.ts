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
import { FaultID } from '../utils/FaultId';
import * as typeUtils from '../utils/TypeUtils';

import type { ConversionLocalState } from './PreScan';

type PrevState = undefined;
type LocalState = ConversionLocalState;

export class TypeConversionTraverser extends VisitorTraverser<PrevState, LocalState> {
  protected removeReservedKeywordIdentifier: boolean;

  constructor(
    removeReservedKeywordIdentifier: boolean,
    context: ts.TransformationContext,
    typeChecker: ts.TypeChecker,
    state: TraverserState<PrevState, LocalState>
  ) {
    super(new DepthFirstAlgorithm(), context, typeChecker, state);
    this.removeReservedKeywordIdentifier = removeReservedKeywordIdentifier;
  }

  protected buildVisitors(): Map<ts.SyntaxKind, ts.Visitor[]> {
    return new Map<ts.SyntaxKind, ts.Visitor[]>([
      [ts.SyntaxKind.LiteralType, [this[FaultID.NumericLiteral].bind(this), this[FaultID.BooleanLiteral].bind(this)]],
      [
        ts.SyntaxKind.PropertyDeclaration,
        [this[FaultID.PropertyAccessExpression].bind(this), this[FaultID.NoInitializer].bind(this)]
      ],
      [ts.SyntaxKind.TypeLiteral, [this[FaultID.TypeLiteral].bind(this)]],
      [
        ts.SyntaxKind.TypeReference,
        [
          this[FaultID.NoBuiltInType].bind(this),
          this[FaultID.NoDefaultAccess].bind(this),
          this[FaultID.UtilityType].bind(this),
          this[FaultID.NoInvalidSDKType].bind(this),
          this[FaultID.WrapperToPrimitive].bind(this)
        ]
      ],
      [ts.SyntaxKind.MappedType, [this[FaultID.MappedType].bind(this)]],
      [ts.SyntaxKind.IntersectionType, [this[FaultID.IntersectionTypeJSValue].bind(this)]],
      [ts.SyntaxKind.SymbolKeyword, [this[FaultID.SymbolToJSValue].bind(this)]],
      [ts.SyntaxKind.AnyKeyword, [this[FaultID.AnyToJSValue].bind(this)]],
      [
        ts.SyntaxKind.TypeAliasDeclaration,
        [
          this[FaultID.RemoveLimitDecorator].bind(this),
          this[FaultID.StringTypeAlias].bind(this),
          this[FaultID.IndexAccessType].bind(this),
          // TODO: This rule should not be here.
          this[FaultID.ConditionalTypes].bind(this)
        ]
      ],
      [ts.SyntaxKind.TypeOperator, [this[FaultID.KeyofType].bind(this)]],
      [ts.SyntaxKind.UnknownKeyword, [this[FaultID.UnknownToJSValue].bind(this)]],
      [
        ts.SyntaxKind.FunctionDeclaration,
        [
          this[FaultID.GeneratorFunction].bind(this),
          this[FaultID.ObjectBindingParams].bind(this),
          this[FaultID.RemoveLimitDecorator].bind(this)
        ]
      ],
      [ts.SyntaxKind.IndexedAccessType, [this[FaultID.IndexAccessType].bind(this)]],
      [
        ts.SyntaxKind.ClassDeclaration,
        [
          this[FaultID.NoReservedKeywordAsIdentifier].bind(this),
          this[FaultID.NoETSKeyword].bind(this),
          this[FaultID.RemoveLimitDecorator].bind(this),
          this[FaultID.NoOptionalMemberMethod].bind(this)
        ]
      ],
      [
        ts.SyntaxKind.InterfaceDeclaration,
        [
          this[FaultID.NoReservedKeywordAsIdentifier].bind(this),
          this[FaultID.NoETSKeyword].bind(this),
          this[FaultID.NoOptionalMemberMethod].bind(this)
        ]
      ],
      [
        ts.SyntaxKind.EnumDeclaration,
        [this[FaultID.NoReservedKeywordAsIdentifier].bind(this), this[FaultID.NoHeterogeneousEnum].bind(this)]
      ],
      [ts.SyntaxKind.TypeParameter, [this[FaultID.LiteralType].bind(this)]],
      [ts.SyntaxKind.GetAccessor, [this[FaultID.NoGetAccessorType].bind(this)]],
      [
        ts.SyntaxKind.SourceFile,
        [
          this[FaultID.NoInvalidSDKImportExport].bind(this),
          this[FaultID.DuplicatedDeclaration].bind(this),
          this[FaultID.DuplicatedEnum].bind(this),
          this[FaultID.NoDefaultAccess].bind(this)
        ]
      ],
      [
        ts.SyntaxKind.ModuleBlock,
        [this[FaultID.DuplicatedDeclaration].bind(this), this[FaultID.DuplicatedEnum].bind(this)]
      ],
      [ts.SyntaxKind.ConstructorType, [this[FaultID.ConstructorType].bind(this)]],
      [ts.SyntaxKind.TemplateLiteralType, [this[FaultID.TemplateLiteralType].bind(this)]],
      [ts.SyntaxKind.VariableDeclaration, [this[FaultID.ConstLiteralToType].bind(this)]],
      [ts.SyntaxKind.FunctionType, [this[FaultID.FunctionType].bind(this)]],
      [ts.SyntaxKind.ImportType, [this[FaultID.NoImportType].bind(this)]],
      [ts.SyntaxKind.TypeQuery, [this[FaultID.TypeQuery].bind(this)]],
      [ts.SyntaxKind.VariableDeclarationList, [this[FaultID.VarDeclarationAssignment].bind(this)]],
      [ts.SyntaxKind.UnionType, [this[FaultID.NoVoidUnionType].bind(this)]],
      [
        ts.SyntaxKind.Parameter,
        [this[FaultID.ReservedFuncParameter].bind(this), this[FaultID.RestParameterArray].bind(this)]
      ],
      [ts.SyntaxKind.VariableStatement, [this[FaultID.NoReservedKeywordAsIdentifier].bind(this)]],
      [ts.SyntaxKind.ImportSpecifier, [this[FaultID.NoETSKeyword].bind(this)]],
      [ts.SyntaxKind.ExportSpecifier, [this[FaultID.NoETSKeyword].bind(this)]],
      [
        ts.SyntaxKind.ExportAssignment,
        [this[FaultID.NoETSKeyword].bind(this), this[FaultID.NoReservedKeywordAsIdentifier].bind(this)]
      ],
      [
        ts.SyntaxKind.ImportDeclaration,
        [this[FaultID.NoEmptyImport].bind(this), this[FaultID.NoLazyImport].bind(this)]
      ],
      [ts.SyntaxKind.ExportDeclaration, [this[FaultID.NoEmptyExport].bind(this)]],
      [ts.SyntaxKind.TupleType, [this[FaultID.TupleTypeToArray].bind(this)]],
      [ts.SyntaxKind.ExpressionWithTypeArguments, [this[FaultID.NoDefaultAccess].bind(this)]]
    ]);
  }

  /**
   * Rule: `arkts-no-number-literal`
   */
  private [FaultID.NumericLiteral](node: ts.Node): ts.Node {
    if (ts.isLiteralTypeNode(node)) {
      const typeNode = node.literal;
      if (
        (ts.isPrefixUnaryExpression(typeNode) && typeNode.operand.kind === ts.SyntaxKind.NumericLiteral) ||
        ts.isNumericLiteral(typeNode)
      ) {
        return this.context.factory.createKeywordTypeNode(ts.SyntaxKind.NumberKeyword);
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-no-boolean-literal`
   */
  private [FaultID.BooleanLiteral](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isLiteralTypeNode(node)) {
      const literal = node.literal;
      if (literal.kind === ts.SyntaxKind.TrueKeyword || literal.kind === ts.SyntaxKind.FalseKeyword) {
        return this.context.factory.createKeywordTypeNode(ts.SyntaxKind.BooleanKeyword);
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-no-type-literal`
   */
  private [FaultID.TypeLiteral](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * TypeLiteral mapped to JSValue in arkts1.2
     */

    if (ts.isTypeLiteralNode(node)) {
      return typeUtils.createJSValueTypeNode(this.context);
    }
    return node;
  }

  /**
   * Rule: `arkts-convert-incompatible-builtins-to-any`
   */
  private [FaultID.NoBuiltInType](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isTypeReferenceNode(node)) {
      if (typeUtils.isIncompatibleBuiltinType(node)) {
        return typeUtils.createJSValueTypeNode(this.context);
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-convert-utility-types-to-replacements`
   */
  private [FaultID.UtilityType](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isTypeReferenceNode(node) && typeUtils.isUtilityType(node)) {
      const typeName = node.typeName;
      if (ts.isIdentifier(typeName)) {
        const target = typeUtils.UTILITY_TYPES.get(typeName.text);
        if (target === typeUtils.JSVALUE) {
          return typeUtils.createJSValueTypeNode(this.context);
        } else if (target === typeUtils.ARRAY_TYPE) {
          return typeUtils.createJSValueArrayTypeNode(this.context);
        } else if (target === typeUtils.STRING) {
          return this.context.factory.createKeywordTypeNode(ts.SyntaxKind.StringKeyword);
        }
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-convert-optional-member-method-to-property`
   */
  private [FaultID.NoOptionalMemberMethod](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Member functions with the optional question token will no longer be supported in ArkTS 1.2.
     * Convert optional methods to optional function properties.
     */
    if (ts.isClassDeclaration(node)) {
      const updatedMembers = node.members.filter(isNotOptionalMemberFunction);
      return this.context.factory.updateClassDeclaration(
        node,
        node.modifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        updatedMembers as ts.PropertyDeclaration[]
      );
    }
    if (ts.isInterfaceDeclaration(node)) {
      const updatedMembers = node.members.filter(isNotOptionalMemberFunction);
      return this.context.factory.updateInterfaceDeclaration(
        node,
        node.modifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        updatedMembers as ts.PropertySignature[]
      );
    }
    return node;
  }

  /**
   * Rule: `arkts-convert-incompatible-sdk-types-to-any`
   */
  private [FaultID.NoInvalidSDKType](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isTypeReferenceNode(node)) {
      const type = this.typeChecker.getTypeAtLocation(node);
      const symbol = type.getSymbol();
      if (!symbol) {
        return node;
      }
      for (const decl of symbol.getDeclarations() || []) {
        const sourceFilePath = decl.getSourceFile().fileName;
        const sourceName = typeUtils.getBaseNameFromFilePath(sourceFilePath);
        if (typeUtils.isIncompatibleSDK(sourceName)) {
          return typeUtils.createJSValueTypeNode(this.context);
        }
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-convert-mappedtype-to-any`
   */
  private [FaultID.MappedType](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * For mapped types, convert them to JSValue
     */
    if (ts.isMappedTypeNode(node)) {
      return typeUtils.createJSValueTypeNode(this.context);
    }

    return node;
  }

  /**
   * Rule: `arkts-convert-intersection-type-to-any`
   */
  private [FaultID.IntersectionTypeJSValue](node: ts.Node): ts.VisitResult<ts.Node> {
    /*
     * Intersection type mapped to JSValue in arkts1.2
     */

    if (ts.isIntersectionTypeNode(node)) {
      return typeUtils.createJSValueTypeNode(this.context);
    }

    return node;
  }

  /**
   * Rule: `arkts-convert-symbol-to-any`
   */
  private [FaultID.SymbolToJSValue](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Keyword 'symbol' mapped to JSValue in arkts1.2
     */

    if (node.kind === ts.SyntaxKind.SymbolKeyword) {
      return typeUtils.createJSValueTypeNode(this.context);
    }

    return node;
  }

  /**
   * Rule: `arkts-convert-any-to-jsvalue`
   */
  private [FaultID.AnyToJSValue](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Keyword 'any' mapped to JSValue in arkts1.2
     */

    if (node.kind === ts.SyntaxKind.AnyKeyword) {
      return typeUtils.createJSValueTypeNode(this.context);
    }

    return node;
  }

  /**
   * Rule: `arkts-convert-conditional-type-to-jsvalue`
   */
  private [FaultID.ConditionalTypes](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * ConditionalTypes mapped to JSValue in arkts1.2
     */

    if (ts.isTypeAliasDeclaration(node)) {
      if (ts.isConditionalTypeNode(node.type)) {
        const newType = typeUtils.createJSValueTypeNode(this.context);
        return ts.factory.createTypeAliasDeclaration(node.modifiers, node.name, node.typeParameters, newType);
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-no-string-type-alias`
   */
  private [FaultID.StringTypeAlias](node: ts.Node): ts.VisitResult<ts.Node> {
    // TODO: Maybe should be deleted.

    /**
     * StringTypeAlias mapped to string in arkts1.2
     */

    if (ts.isTypeAliasDeclaration(node)) {
      if (ts.isLiteralTypeNode(node.type)) {
        let isStringLiteral = false;
        node.type.forEachChild((child: ts.Node) => {
          if (child.kind === ts.SyntaxKind.StringLiteral) {
            isStringLiteral = true;
            return;
          }
        });
        if (isStringLiteral) {
          return ts.factory.createTypeAliasDeclaration(
            node.modifiers,
            node.name,
            node.typeParameters,
            ts.factory.createKeywordTypeNode(ts.SyntaxKind.StringKeyword)
          );
        }
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-no-indexed-access-type`
   */
  private [FaultID.IndexAccessType](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * IndexedAccessType mapped to JSValue in arkts1.2
     */

    if (ts.isTypeAliasDeclaration(node) && ts.isIndexedAccessTypeNode(node.type)) {
      const type = this.typeChecker.getTypeAtLocation(node.type);
      const newType = this.typeChecker.typeToTypeNode(type, undefined, ts.NodeBuilderFlags.NoTruncation);
      if (newType) {
        // Create a new type alias declaration node
        return ts.factory.createTypeAliasDeclaration(node.modifiers, node.name, node.typeParameters, newType);
      }
    } else if (ts.isIndexedAccessTypeNode(node) && !ts.isLiteralTypeNode(node.indexType)) {
      return typeUtils.createJSValueTypeNode(this.context);
    }

    return node;
  }

  /**
   * Rule: `arkts-convert-type-query-to-jsvalue`
   */
  private [FaultID.TypeQuery](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * TypeQuery mapped to Reflect in arkts1.2
     */
    const identifiers = new Set(['Reflect', 'Promise']);
    if (ts.isTypeQueryNode(node) && ts.isIdentifier(node.exprName)) {
      if (identifiers.has(node.exprName.text)) {
        return this.context.factory.createTypeReferenceNode(node.exprName.text, undefined);
      } else {
        return typeUtils.createJSValueTypeNode(this.context);
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-convert-keyof-type-to-jsvalue`
   */
  private [FaultID.KeyofType](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * keyof result in TS function mapped to JSValue in arkts1.2
     */

    if (ts.isTypeOperatorNode(node)) {
      return typeUtils.createJSValueTypeNode(this.context);
    }
    return node;
  }

  /**
   * Rule: `arkts-convert-unknown-to-jsvalue`
   */
  private [FaultID.UnknownToJSValue](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Keyword 'unknown' mapped to JSValue in arkts1.2
     */

    if (node.kind === ts.SyntaxKind.UnknownKeyword) {
      return typeUtils.createJSValueTypeNode(this.context);
    }

    return node;
  }

  /**
   * Rule: `arkts-no-wrapperTypes`
   */
  private [FaultID.WrapperToPrimitive](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Wrapper types mapped to primitive type in arkts1.2
     */
    if (ts.isTypeReferenceNode(node)) {
      const identifier = node.typeName;
      if (ts.isIdentifier(identifier)) {
        if (identifier.text !== undefined && identifier.text === 'Wrapper') {
          return typeUtils.createJSValueTypeNode(this.context);
        }
        switch (identifier.text) {
          case 'Number':
            return this.context.factory.createKeywordTypeNode(ts.SyntaxKind.NumberKeyword);
          case 'String':
            return this.context.factory.createKeywordTypeNode(ts.SyntaxKind.StringKeyword);
          case 'Boolean':
            return this.context.factory.createKeywordTypeNode(ts.SyntaxKind.BooleanKeyword);
          case 'BigInt':
            return this.context.factory.createKeywordTypeNode(ts.SyntaxKind.BigIntKeyword);
          default:
            return node;
        }
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-no-generator-function`
   */
  private [FaultID.GeneratorFunction](node: ts.Node): ts.VisitResult<ts.Node> {
    void this;

    /**
     * GeneratorFunction mapped to JSValue in arkts1.2
     */

    if (ts.isFunctionDeclaration(node) && node.type) {
      const returnType = node.type;
      if (
        ts.isTypeReferenceNode(returnType) &&
        ts.isIdentifier(returnType.typeName) &&
        returnType.typeName.text === 'Generator'
      ) {
        const newType = typeUtils.createJSValueTypeNode(this.context);
        return ts.factory.updateFunctionDeclaration(
          node,
          node.modifiers,
          undefined,
          node.name,
          node.typeParameters,
          node.parameters,
          newType,
          undefined
        );
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-object-bind-params`
   */
  private [FaultID.ObjectBindingParams](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * ObjectBindingParams mapped to JSValue in arkts1.2
     */

    if (ts.isFunctionDeclaration(node)) {
      const parameters = node.parameters;
      const newParameters: ts.ParameterDeclaration[] = [];
      if (parameters !== undefined && parameters.length > 0) {
        parameters.forEach((parameter, index) => {
          if (ts.isObjectBindingPattern(parameter.name)) {
            const typeNode = typeUtils.createJSValueTypeNode(this.context);
            const currentParameter = this.context.factory.createParameterDeclaration(
              undefined,
              undefined,
              this.context.factory.createIdentifier(`args${index}`),
              undefined,
              typeNode
            );
            newParameters.push(currentParameter);
          } else {
            newParameters.push(parameter);
          }
        });

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
    }

    return node;
  }

  /**
   * Rule: `arkts-remove-limit-decorator`
   */
  private [FaultID.RemoveLimitDecorator](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!ts.isFunctionDeclaration(node) && !ts.isClassDeclaration(node) && !ts.isTypeAliasDeclaration(node)) {
      return node;
    }

    let decorators: ts.Decorator[] = [];
    const illegalDecorators = ts.getAllDecorators(node);
    decorators = illegalDecorators?.filter(ts.isDecorator) as ts.Decorator[];
    // Filter out restricted decorators
    const filteredDecorators = decorators?.filter((decorator) => {
      const expression = decorator.expression;
      return !(ts.isIdentifier(expression) && typeUtils.LIMIT_DECORATOR.includes(expression.text));
    });
    (node as unknown as { illegalDecorators: ts.Decorator[] }).illegalDecorators = filteredDecorators;
    if (ts.isClassDeclaration(node)) {
      const existingModifiers = node.modifiers?.filter((m) => !ts.isDecorator(m)) || [];
      const newModifiers: ts.ModifierLike[] = [...filteredDecorators, ...existingModifiers];

      return this.context.factory.updateClassDeclaration(
        node,
        newModifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        node.members
      );
    }

    return node;
  }

  /**
   * Rule: `arkts-no-initializer`
   */
  private [FaultID.NoInitializer](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * For member variables with initial assignment, convert literals to their corresponding data types.
     */

    if (!ts.isPropertyDeclaration(node)) {
      return node;
    }

    const nodeType = inferNodeTypeFromInitializer(node, this.context);

    if (nodeType !== undefined) {
      return this.context.factory.updatePropertyDeclaration(
        node,
        node.modifiers,
        node.name,
        undefined,
        nodeType,
        undefined
      );
    }

    return node;
  }

  /**
   * Rule: `arkts-no-literal-type`
   */
  private [FaultID.LiteralType](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Union literal type mapped to JSValue in arkts1.2
     */
    if (ts.isTypeParameterDeclaration(node)) {
      const { unionTypeNode, typeNode } = extractTypeNodes(node, this.context);

      if (unionTypeNode === undefined && typeNode !== undefined) {
        return createTypeParameterDeclaration(node, undefined, typeNode, this.context);
      } else if (unionTypeNode !== undefined && typeNode === undefined) {
        return createTypeParameterDeclaration(node, unionTypeNode, undefined, this.context);
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-get-accessor-type`
   */
  private [FaultID.NoGetAccessorType](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * For get accessors without type annotation, add 'Any' type.
     * e.g. get name() {} => get name(): Any {}
     */

    if (ts.isGetAccessorDeclaration(node)) {
      if (node.type === undefined) {
        const anyType = typeUtils.createJSValueTypeNode(this.context);
        return this.context.factory.updateGetAccessorDeclaration(
          node,
          node.modifiers,
          node.name,
          node.parameters,
          anyType,
          node.body
        );
      }
    }

    return node;
  }

  /*
   * Rule: `arkts-no-heterogeneous-enum`
   */
  private [FaultID.NoHeterogeneousEnum](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isEnumDeclaration(node)) {
      if (!isHeterogeneousEnum(node)) {
        return node;
      }

      const updatedMembers = node.members.map((member) => {
        if (!member.initializer) {
          return member;
        }

        return this.context.factory.updateEnumMember(member, member.name, undefined);
      });

      return this.context.factory.updateEnumDeclaration(node, node.modifiers, node.name, updatedMembers);
    }

    return node;
  }

  /**
   * Rule: `arkts-no-constructor-type`
   */
  private [FaultID.ConstructorType](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isConstructorTypeNode(node)) {
      return typeUtils.createJSValueTypeNode(this.context);
    }
    return node;
  }

  /**
   * Rule: `arkts-no-template-literal-type`
   */
  private [FaultID.TemplateLiteralType](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isTemplateLiteralTypeNode(node)) {
      return this.context.factory.createKeywordTypeNode(ts.SyntaxKind.StringKeyword);
    }
    return node;
  }

  /*
   * Rule: `arkts-const-literal-to-type`
   */
  private [FaultID.ConstLiteralToType](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Convert const variable declarations with literal values to type declarations.
     * e.g. export declare const c = 123; -> export declare const c: number;
     */

    if (ts.isVariableDeclaration(node) && node.initializer) {
      let typeNode: ts.TypeNode | undefined;

      if (ts.isNumericLiteral(node.initializer)) {
        typeNode = this.context.factory.createKeywordTypeNode(ts.SyntaxKind.NumberKeyword);
      }

      if (typeNode) {
        const result = this.context.factory.createVariableDeclaration(
          node.name,
          node.exclamationToken,
          typeNode,
          undefined
        );

        return result;
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-generic-function-type`
   */
  private [FaultID.FunctionType](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * For function types with generic parameters, convert them to Any
     */
    if (ts.isFunctionTypeNode(node)) {
      if (node.typeParameters && node.typeParameters.length > 0) {
        return convertGenericTypeInLambdaIntoAny(node, this.context);
      }
    }

    return node;
  }

  /*
   * Rule: `Convert import type to Any`
   */
  private [FaultID.NoImportType](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isImportTypeNode(node)) {
      return typeUtils.createJSValueTypeNode(this.context);
    }
    return node;
  }

  /**
   * Rule: `arkts-no-bigint-binaryExpression`
   */
  private [FaultID.VarDeclarationAssignment](node: ts.Node): ts.VisitResult<ts.Node> {
    /*
     * bigint literal map to number,and binary expression map to boolean in arkts1.2
     */
    // TODO: Consider other numeric literals

    if (ts.isVariableDeclarationList(node)) {
      const isLetDeclaration = node.flags & ts.NodeFlags.Let;
      const isConstDeclaration = node.flags & ts.NodeFlags.Const;

      // update boolean type declaration function
      if (isConstDeclaration || isLetDeclaration) {
        return transformLogicalOperators(
          node,
          this.context,
          isConstDeclaration ? ts.NodeFlags.Const : ts.NodeFlags.Let
        );
      }
    }

    return node;
  }

  /*
   * Rule: `arkts-no-property-access-expression`
   */
  private [FaultID.PropertyAccessExpression](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Property access expression convert to specific type
     */
    if (ts.isPropertyDeclaration(node)) {
      const initializer = node.initializer;
      if (!initializer || initializer.kind !== ts.SyntaxKind.PropertyAccessExpression) {
        return node;
      }

      const updatedInitializer = updatePropertyAccessExpression(
        initializer as ts.PropertyAccessExpression,
        this.context
      );
      if (updatedInitializer) {
        return this.context.factory.updatePropertyDeclaration(
          node,
          node.modifiers,
          node.name,
          node.questionToken,
          updatedInitializer,
          undefined
        );
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-no-invalid-sdk-import`
   */
  private [FaultID.NoInvalidSDKImportExport](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isSourceFile(node)) {
      const statements: ts.Statement[] = [];
      for (const stmt of node.statements) {
        if (ts.isImportDeclaration(stmt)) {
          const filtered = filterImportDeclarationFromInvalidSDK(stmt, this.typeChecker, node, this.context);
          if (filtered) {
            statements.push(filtered);
          }
        } else if (ts.isExportDeclaration(stmt)) {
          const filtered = filterExportDeclarationFromInvalidSDK(stmt, this.typeChecker, node, this.context);
          if (filtered) {
            statements.push(filtered);
          }
        } else if (ts.isExportAssignment(stmt)) {
          const filtered = filterExportAssignmentFromInvalidSDK(stmt, this.typeChecker, node);
          if (filtered) {
            statements.push(filtered);
          }
        } else {
          statements.push(stmt);
        }
      }
      return this.context.factory.updateSourceFile(node, statements);
    }
    return node;
  }

  /**
   * Rule: `arkts-convert-union-type-with-void-to-jsvalue`
   */
  private [FaultID.NoVoidUnionType](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * If a union type contains the void type,
     * convert the union type to Any.
     */
    if (ts.isUnionTypeNode(node)) {
      const hasVoid = node.types.some((type) => type.kind === ts.SyntaxKind.VoidKeyword);
      if (hasVoid) {
        return typeUtils.createJSValueTypeNode(this.context);
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-reserved-function-parameter-name`
   */
  private [FaultID.ReservedFuncParameter](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Some of the function parameters are reserved keywords in ArkTS 1.2.
     * decorate them with underscore prefix to avoid conflicts.
     */
    if (ts.isParameter(node) && ts.isIdentifier(node.name) && typeUtils.isHardOrReservedKeyword(node.name.text)) {
      const updatedNode = this.context.factory.updateParameterDeclaration(
        node,
        node.modifiers,
        node.dotDotDotToken,
        this.context.factory.createIdentifier(`_${node.name.text}`),
        node.questionToken,
        node.type,
        node.initializer
      );
      return updatedNode;
    }
    return node;
  }

  /**
   * Rule: `arkts-remove-reserved-keyword-identifier`
   */
  private [FaultID.NoReservedKeywordAsIdentifier](node: ts.Node): ts.VisitResult<ts.Node> {
    if (!this.removeReservedKeywordIdentifier) {
      return node;
    }
    if (ts.isClassDeclaration(node)) {
      const filteredMembers = node.members.filter((member) => !isReservedKeywordMember(member));
      return this.context.factory.updateClassDeclaration(
        node,
        node.modifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        filteredMembers
      );
    }
    if (ts.isInterfaceDeclaration(node)) {
      const filteredMembers = node.members.filter((member) => !isReservedKeywordMember(member));
      return this.context.factory.updateInterfaceDeclaration(
        node,
        node.modifiers,
        node.name,
        node.typeParameters,
        node.heritageClauses,
        filteredMembers
      );
    }
    if (ts.isEnumDeclaration(node)) {
      const filteredMembers = node.members.filter((member) => {
        const name = member.name;
        return !(ts.isIdentifier(name) && typeUtils.isHardOrReservedKeyword(name.text));
      });
      return this.context.factory.updateEnumDeclaration(node, node.modifiers, node.name, filteredMembers);
    }
    if (ts.isVariableStatement(node)) {
      const decls = node.declarationList.declarations;
      const kept = decls.filter((d) => !(ts.isIdentifier(d.name) && typeUtils.isHardOrReservedKeyword(d.name.text)));
      if (kept.length === 0) {
        return undefined;
      }
      if (kept.length < decls.length) {
        const newList = this.context.factory.createVariableDeclarationList(kept, node.declarationList.flags);
        return this.context.factory.updateVariableStatement(node, node.modifiers, newList);
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-no-etskeyword`
   */
  private [FaultID.NoETSKeyword](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isClassDeclaration(node) || ts.isInterfaceDeclaration(node)) {
      if (node.parent && !ts.isSourceFile(node.parent)) {
        return node;
      }
      const name = node.name;
      if (name && ts.isIdentifier(name) && typeUtils.ETS_KEYWORDS.has(name.text)) {
        return undefined;
      }
    }
    if (ts.isImportSpecifier(node) || ts.isExportSpecifier(node)) {
      const name = node.name;
      if (name && ts.isIdentifier(name) && typeUtils.ETS_KEYWORDS.has(name.text)) {
        return undefined;
      }
    }
    if (ts.isExportAssignment(node)) {
      const expr = node.expression;
      if (ts.isIdentifier(expr) && typeUtils.ETS_KEYWORDS.has(expr.text)) {
        return undefined;
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-tuple-type-to-array`
   */
  private [FaultID.TupleTypeToArray](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isTupleTypeNode(node)) {
      const arrayType = typeUtils.createJSValueArrayTypeReferenceNode(this.context);
      return arrayType;
    }
    return node;
  }

  /**
   * Rule: `arkts-no-empty-import`
   */
  private [FaultID.NoEmptyImport](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isImportDeclaration(node)) {
      const importClause = node.importClause;
      if (importClause?.namedBindings && ts.isNamedImports(importClause.namedBindings)) {
        if (importClause.namedBindings.elements.length === 0) {
          return undefined;
        }
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-no-lazy-import`
   */
  private [FaultID.NoLazyImport](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isImportDeclaration(node)) {
      const importClause = node.importClause;
      if (importClause && importClause.isLazy) {
        const newImportClause = this.context.factory.createImportClause(
          importClause.isTypeOnly,
          importClause.name,
          importClause.namedBindings
        );
        return this.context.factory.updateImportDeclaration(
          node,
          node.modifiers,
          newImportClause,
          node.moduleSpecifier,
          node.assertClause
        );
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-no-empty-export`
   */
  private [FaultID.NoEmptyExport](node: ts.Node): ts.VisitResult<ts.Node> {
    if (ts.isExportDeclaration(node)) {
      const exportClause = node.exportClause;
      if (exportClause && ts.isNamedExports(exportClause)) {
        if (exportClause.elements.length === 0) {
          return undefined;
        }
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-no-duplicated-declaration`
   */
  private [FaultID.DuplicatedDeclaration](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Duplicate interface and class declarations will be merged into one.
     */

    if (!ts.isSourceFile(node) && !ts.isModuleBlock(node)) {
      return node;
    }
    const statements: ts.Statement[] = [];
    const interfaceDeclarations: ts.InterfaceDeclaration[] = [];
    const classDeclarations: ts.ClassDeclaration[] = [];
    for (const stmt of node.statements) {
      if (ts.isInterfaceDeclaration(stmt)) {
        interfaceDeclarations.push(stmt);
      } else if (ts.isClassDeclaration(stmt)) {
        classDeclarations.push(stmt);
      } else {
        statements.push(stmt);
      }
    }
    if (ts.isModuleBlock(node)) {
      const moduleBlockNode = this.context.factory.updateModuleBlock(node, [
        ...statements,
        ...findSameInterfaceAndClassList(interfaceDeclarations, this.context, ts.SyntaxKind.InterfaceDeclaration),
        ...findSameInterfaceAndClassList(classDeclarations, this.context, ts.SyntaxKind.ClassDeclaration)
      ]);
      return moduleBlockNode;
    } else {
      return this.context.factory.updateSourceFile(node, [
        ...statements,
        ...findSameInterfaceAndClassList(interfaceDeclarations, this.context, ts.SyntaxKind.InterfaceDeclaration),
        ...findSameInterfaceAndClassList(classDeclarations, this.context, ts.SyntaxKind.ClassDeclaration)
      ]);
    }
  }

  /**
   * Rule: `arkts-rest-parameter-array`
   */
  private [FaultID.RestParameterArray](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * Rest parameters named `ESObject` will be converted to `Any[]` type in ArkTS 1.2.
     */
    if (ts.isParameter(node) && node.dotDotDotToken && node.type && ts.isTypeReferenceNode(node.type)) {
      const typeName = node.type.typeName;
      if (ts.isIdentifier(typeName) && (typeName.text === typeUtils.ESOBJECT || typeName.text === typeUtils.JSVALUE)) {
        const typeNode = typeUtils.createJSValueArrayTypeNode(this.context);
        return this.context.factory.updateParameterDeclaration(
          node,
          node.modifiers,
          node.dotDotDotToken,
          node.name,
          node.questionToken,
          typeNode,
          node.initializer
        );
      }
    }
    return node;
  }

  /**
   * Rule: `arkts-no-duplicated-enum`
   */
  private [FaultID.DuplicatedEnum](node: ts.Node): ts.VisitResult<ts.Node> {
    /**
     * For duplicated enums, convert them to one enum
     */

    const statements: ts.Statement[] = [];
    const interfaceDeclarations: ts.InterfaceDeclaration[] = [];
    const classDeclarations: ts.ClassDeclaration[] = [];
    const enumDeclarations: ts.EnumDeclaration[] = [];
    if (ts.isSourceFile(node) || ts.isModuleBlock(node)) {
      for (const stmt of node.statements) {
        if (ts.isInterfaceDeclaration(stmt)) {
          interfaceDeclarations.push(stmt);
        } else if (ts.isClassDeclaration(stmt)) {
          classDeclarations.push(stmt);
        } else if (ts.isEnumDeclaration(stmt)) {
          enumDeclarations.push(stmt);
        } else {
          statements.push(stmt);
        }
      }
      if (ts.isModuleBlock(node)) {
        const moduleBlockNode = this.context.factory.updateModuleBlock(node, [
          ...statements,
          ...findSameInterfaceOrClassOrEnumList(
            interfaceDeclarations,
            this.context,
            ts.SyntaxKind.InterfaceDeclaration
          ),
          ...findSameInterfaceOrClassOrEnumList(classDeclarations, this.context, ts.SyntaxKind.ClassDeclaration),
          ...findSameInterfaceOrClassOrEnumList(enumDeclarations, this.context, ts.SyntaxKind.EnumDeclaration)
        ]);
        return moduleBlockNode;
      } else {
        return this.context.factory.updateSourceFile(node, [
          ...statements,
          ...findSameInterfaceOrClassOrEnumList(
            interfaceDeclarations,
            this.context,
            ts.SyntaxKind.InterfaceDeclaration
          ),
          ...findSameInterfaceOrClassOrEnumList(classDeclarations, this.context, ts.SyntaxKind.ClassDeclaration),
          ...findSameInterfaceOrClassOrEnumList(enumDeclarations, this.context, ts.SyntaxKind.EnumDeclaration)
        ]);
      }
    }

    return node;
  }

  /**
   * Rule: `arkts-convert-default-access-into-default-import`
   */
  private [FaultID.NoDefaultAccess](node: ts.Node): ts.VisitResult<ts.Node> {
    /** Handles three node kinds:
     * - TypeReference: converts `Ns.default` to a generated identifier
     * - ExpressionWithTypeArguments: converts `extends/implements Ns.default` to a generated identifier
     * - SourceFile: inserts the collected default import declarations
     */
    if (ts.isTypeReferenceNode(node)) {
      return convertDefaultAccessTypeRef(node, this.typeChecker, this.state.localState, this.context);
    }
    if (ts.isExpressionWithTypeArguments(node)) {
      return convertDefaultAccessExprWithTypeArgs(node, this.typeChecker, this.state.localState, this.context);
    }
    if (ts.isSourceFile(node)) {
      return insertPendingDefaultImports(node, this.state.localState, this.context);
    }
    return node;
  }
}

/**
 * Convert a `Ns.default` type reference to a generated identifier,
 * recording the pending default import for later insertion.
 */
function convertDefaultAccessTypeRef(
  node: ts.TypeReferenceNode,
  typeChecker: ts.TypeChecker,
  localState: ConversionLocalState,
  context: ts.TransformationContext
): ts.VisitResult<ts.Node> {
  const typeName = node.typeName;
  if (!ts.isQualifiedName(typeName) || typeName.right.text !== 'default') {
    return node;
  }

  const left = typeName.left;
  if (!ts.isIdentifier(left)) {
    return node;
  }

  const namespaceName = left.text;

  // Reuse previously resolved name for the same namespace
  if (localState.defaultAccessMap.has(namespaceName)) {
    return context.factory.updateTypeReferenceNode(
      node,
      context.factory.createIdentifier(localState.defaultAccessMap.get(namespaceName)!),
      node.typeArguments
    );
  }

  const moduleSpecifier = resolveNamespaceModuleSpecifier(left, typeChecker);
  if (!moduleSpecifier) {
    return node;
  }

  const generatedName = generateUniqueDefaultName(namespaceName, localState.existingTypeNames);
  localState.defaultAccessMap.set(namespaceName, generatedName);
  localState.pendingDefaultImports.set(generatedName, moduleSpecifier);

  return context.factory.updateTypeReferenceNode(
    node,
    context.factory.createIdentifier(generatedName),
    node.typeArguments
  );
}

/**
 * Convert a `extends/implements Ns.default` expression-with-type-arguments to a generated identifier,
 * recording the pending default import for later insertion.
 */
function convertDefaultAccessExprWithTypeArgs(
  node: ts.ExpressionWithTypeArguments,
  typeChecker: ts.TypeChecker,
  localState: ConversionLocalState,
  context: ts.TransformationContext
): ts.VisitResult<ts.Node> {
  const expr = node.expression;
  if (!ts.isPropertyAccessExpression(expr) || expr.name.text !== 'default') {
    return node;
  }

  const obj = expr.expression;
  if (!ts.isIdentifier(obj)) {
    return node;
  }

  const namespaceName = obj.text;

  if (localState.defaultAccessMap.has(namespaceName)) {
    return context.factory.updateExpressionWithTypeArguments(
      node,
      context.factory.createIdentifier(localState.defaultAccessMap.get(namespaceName)!),
      node.typeArguments
    );
  }

  const moduleSpecifier = resolveNamespaceModuleSpecifier(obj, typeChecker);
  if (!moduleSpecifier) {
    return node;
  }

  const generatedName = generateUniqueDefaultName(namespaceName, localState.existingTypeNames);
  localState.defaultAccessMap.set(namespaceName, generatedName);
  localState.pendingDefaultImports.set(generatedName, moduleSpecifier);

  return context.factory.updateExpressionWithTypeArguments(
    node,
    context.factory.createIdentifier(generatedName),
    node.typeArguments
  );
}

/**
 * Resolve the module specifier from a namespace import identifier.
 */
function resolveNamespaceModuleSpecifier(id: ts.Identifier, typeChecker: ts.TypeChecker): string | undefined {
  const symbol = typeChecker.getSymbolAtLocation(id);
  if (!symbol) {
    return undefined;
  }

  for (const decl of symbol.getDeclarations() ?? []) {
    if (ts.isNamespaceImport(decl)) {
      const importDecl = decl.parent.parent;
      if (ts.isImportDeclaration(importDecl) && ts.isStringLiteral(importDecl.moduleSpecifier)) {
        return importDecl.moduleSpecifier.text;
      }
    }
  }
  return undefined;
}

/**
 * Generate a unique `__<name>_default` identifier, prepending `_` until no conflict.
 */
function generateUniqueDefaultName(namespaceName: string, existingNames: Set<string>): string {
  let name = `__${namespaceName}_default`;
  while (existingNames.has(name)) {
    name = `_${name}`;
  }
  existingNames.add(name);
  return name;
}

/**
 * Insert pending default import declarations at the top of a SourceFile.
 */
function insertPendingDefaultImports(
  node: ts.SourceFile,
  localState: ConversionLocalState,
  context: ts.TransformationContext
): ts.VisitResult<ts.Node> {
  if (localState.pendingDefaultImports.size === 0) {
    return node;
  }

  const newImports: ts.ImportDeclaration[] = [];
  for (const [name, moduleSpecifier] of localState.pendingDefaultImports) {
    newImports.push(
      context.factory.createImportDeclaration(
        undefined,
        context.factory.createImportClause(false, context.factory.createIdentifier(name), undefined),
        context.factory.createStringLiteral(moduleSpecifier)
      )
    );
  }

  const oldStatements = node.statements.filter((stmt) => {
    return !isReplacedNamespaceImport(stmt, localState, node);
  });
  return context.factory.updateSourceFile(node, [...newImports, ...oldStatements]);
}

function isReplacedNamespaceImport(
  stmt: ts.Statement,
  localState: ConversionLocalState,
  sourceFile: ts.SourceFile
): boolean {
  if (!ts.isImportDeclaration(stmt)) {
    return false;
  }

  const namespaceName = getNamespaceImportName(stmt);
  return (
    namespaceName !== undefined &&
    localState.defaultAccessMap.has(namespaceName) &&
    !hasRemainingIdentifierUse(sourceFile, stmt, namespaceName)
  );
}

function getNamespaceImportName(stmt: ts.ImportDeclaration): string | undefined {
  const bindings = stmt.importClause?.namedBindings;
  return bindings !== undefined && ts.isNamespaceImport(bindings) ? bindings.name.text : undefined;
}

function hasRemainingIdentifierUse(
  sourceFile: ts.SourceFile,
  ignoredImport: ts.ImportDeclaration,
  name: string
): boolean {
  let found = false;
  const visit = (node: ts.Node): void => {
    if (found || node === ignoredImport) {
      return;
    }
    if (ts.isIdentifier(node) && node.text === name) {
      found = true;
      return;
    }
    node.forEachChild(visit);
  };
  sourceFile.forEachChild(visit);
  return found;
}

/**
 * Extract interface members together.
 */
function extractInterfaceMembers(item: ts.InterfaceDeclaration, methods: ts.TypeElement[]): void {
  item.members.forEach((member) => {
    if ((ts.isMethodSignature(member) || ts.isPropertySignature(member)) && member.name) {
      const memberName = (member.name as ts.Identifier).text;
      if (
        !methods.some(
          (m) =>
            isSameFunction(m, member) ||
            (ts.isPropertySignature(m) && ts.isPropertySignature(member) && m.name.getText() === memberName)
        )
      ) {
        methods.push(member);
      }
    }
  });
}

/**
 * Extract class members together.
 */
function extractClassMembers(item: ts.ClassDeclaration, classMethods: ts.ClassElement[]): void {
  item.members.forEach((member) => {
    if ((ts.isMethodDeclaration(member) || ts.isPropertyDeclaration(member)) && member.name) {
      const memberName = (member.name as ts.Identifier).text;
      if (
        !classMethods.some(
          (m) =>
            (ts.isMethodDeclaration(m) && ts.isMethodDeclaration(member) && m.name.getText() === memberName) ||
            (ts.isPropertyDeclaration(m) && ts.isPropertyDeclaration(member) && m.name.getText() === memberName)
        )
      ) {
        classMethods.push(member);
      }
    }
  });
}

function isSameFunction(m: ts.TypeElement, member: ts.TypeElement): boolean {
  const memberName = (member.name as ts.Identifier).text;
  const mName = (m.name as ts.Identifier).text;

  if (mName !== memberName) {
    return false;
  }
  if (!ts.isMethodSignature(m) || !ts.isMethodSignature(member)) {
    return false;
  }
  if (!compareParameters(m.parameters, member.parameters)) {
    return false;
  }
  if (m.type && member.type) {
    if (!typesAreEqual(m.type, member.type)) {
      return false;
    }
  } else if (!!m.type !== !!member.type) {
    return false;
  }
  if (!compareTypeParameters(m.typeParameters, member.typeParameters)) {
    return false;
  }
  return true;
}

function compareParameters(
  params1: ts.NodeArray<ts.ParameterDeclaration>,
  params2: ts.NodeArray<ts.ParameterDeclaration>
): boolean {
  if (params1.length !== params2.length) {
    return false;
  }
  for (let i = 0; i < params1.length; i++) {
    const p1 = params1[i];
    const p2 = params2[i];
    if (p1.name.getText() !== p2.name.getText()) {
      return false;
    }
    if (!typesAreEqual(p1.type, p2.type)) {
      return false;
    }

    if (p1.questionToken !== p2.questionToken || p1.initializer !== p2.initializer) {
      return false;
    }
  }

  return true;
}

function typesAreEqual(type1: ts.TypeNode | undefined, type2: ts.TypeNode | undefined): boolean {
  if (!type1 || !type2) {
    return type1 === type2;
  }

  return type1.getText() === type2.getText();
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

/**
 * Find a list of interfaces or classes with the same name
 */
function findSameInterfaceAndClassList(
  statements: ts.Statement[],
  context: ts.TransformationContext,
  kindType: number
): ts.Statement[] {
  const sameInterfaceMap = new Map<string, { count: number; nodes: ts.Statement[] }>();
  statements.forEach((statement) => {
    let name: string | undefined;
    const isInterfaceDeclaration = ts.isInterfaceDeclaration(statement) && statement.name;
    const isClassDeclaration = ts.isClassDeclaration(statement) && statement.name;
    if (isInterfaceDeclaration || isClassDeclaration) {
      name = statement.name.text;
    }
    if (name) {
      if (!sameInterfaceMap.has(name)) {
        sameInterfaceMap.set(name, { count: 0, nodes: [] });
      }
      const entry = sameInterfaceMap.get(name)!;
      entry.count++;
      entry.nodes.push(statement);
    }
  });
  const uniqueNodes: ts.Statement[] = [];
  const newNodes: ts.Statement[] = [];
  sameInterfaceMap.forEach((entry) => {
    if (entry.count === 1) {
      uniqueNodes.push(...entry.nodes);
    } else {
      // Reorganize interface information
      const newInterface = createCombinedInterfaceAndClass(entry.nodes, context, kindType);
      newNodes.push(newInterface);
    }
  });

  return [...uniqueNodes, ...newNodes];
}

/**
 * Extract enum members together.
 */
function extractEnumMembers(item: ts.EnumDeclaration, enumbers: ts.EnumMember[]): void {
  item.members.forEach((member) => {
    if (ts.isEnumMember(member) && member.name) {
      const memberName = (member.name as ts.Identifier).text;
      if (!enumbers.some((m) => ts.isEnumMember(m) && ts.isEnumMember(member) && m.name.getText() === memberName)) {
        enumbers.push(member);
      }
    }
  });
}

/**
 * Merge classes or interfaces or enum with the same name
 */
function createCombinedInterfaceAndClass(
  list: ts.Statement[],
  context: ts.TransformationContext,
  kindType: ts.SyntaxKind
): ts.Statement {
  const methods: ts.TypeElement[] = [];
  const classMethods: ts.ClassElement[] = [];
  const enumMembers: ts.EnumMember[] = [];

  list.forEach((item) => {
    if (ts.isInterfaceDeclaration(item)) {
      extractInterfaceMembers(item, methods);
    } else if (ts.isClassDeclaration(item)) {
      extractClassMembers(item, classMethods);
    } else if (ts.isEnumDeclaration(item)) {
      extractEnumMembers(item, enumMembers);
    }
  });

  if (kindType === ts.SyntaxKind.ClassDeclaration) {
    const classNode = list[0] as ts.ClassDeclaration;
    return context.factory.createClassDeclaration(
      classNode.modifiers,
      classNode.name,
      undefined,
      undefined,
      classMethods
    );
  } else if (kindType === ts.SyntaxKind.EnumDeclaration) {
    const enumNode = list[0] as ts.EnumDeclaration;
    return context.factory.createEnumDeclaration(enumNode.modifiers, enumNode.name, enumMembers);
  } else {
    const node = list[0] as ts.InterfaceDeclaration;
    return context.factory.createInterfaceDeclaration(node.modifiers, node.name, undefined, undefined, methods);
  }
}

/**
 * Find a list of interfaces or classes or enums with the same name
 */
function findSameInterfaceOrClassOrEnumList(
  statements: ts.Statement[],
  context: ts.TransformationContext,
  kindType: ts.SyntaxKind
): ts.Statement[] {
  const sameInterfaceMap = new Map<string, { count: number; nodes: ts.Statement[] }>();
  statements.forEach((statement) => {
    let name: string | undefined;
    const isInterfaceDeclaration = ts.isInterfaceDeclaration(statement) && statement.name;
    const isClassDeclaration = ts.isClassDeclaration(statement) && statement.name;
    const isEnumDeclaration = ts.isEnumDeclaration(statement) && statement.name;
    if (isInterfaceDeclaration || isClassDeclaration || isEnumDeclaration) {
      name = statement.name.text;
    }
    if (name) {
      if (!sameInterfaceMap.has(name)) {
        sameInterfaceMap.set(name, { count: 0, nodes: [] });
      }
      const entry = sameInterfaceMap.get(name)!;
      entry.count++;
      entry.nodes.push(statement);
    }
  });
  const uniqueNodes: ts.Statement[] = [];
  const newNodes: ts.Statement[] = [];
  sameInterfaceMap.forEach((entry) => {
    if (entry.count === 1) {
      uniqueNodes.push(...entry.nodes);
    } else {
      // Reorganize interface information
      const newInterface = createCombinedInterfaceAndClass(entry.nodes, context, kindType);
      newNodes.push(newInterface);
    }
  });

  return [...uniqueNodes, ...newNodes];
}

/**
 * Handle initializes are not allowed in the environment content
 */
function inferNodeTypeFromInitializer(
  node: ts.PropertyDeclaration,
  context: ts.TransformationContext
): ts.TypeNode | undefined {
  if (node.initializer !== undefined) {
    switch (node.initializer.kind) {
      case ts.SyntaxKind.FalseKeyword:
      case ts.SyntaxKind.TrueKeyword:
        return context.factory.createKeywordTypeNode(ts.SyntaxKind.BooleanKeyword);
      case ts.SyntaxKind.NumericLiteral:
        return context.factory.createKeywordTypeNode(ts.SyntaxKind.NumberKeyword);
      case ts.SyntaxKind.BigIntLiteral:
        return context.factory.createKeywordTypeNode(ts.SyntaxKind.BigIntKeyword);
      case ts.SyntaxKind.StringLiteral:
        return context.factory.createKeywordTypeNode(ts.SyntaxKind.StringKeyword);
      default:
        return undefined;
    }
  } else if (node.type !== undefined && ts.isLiteralTypeNode(node.type)) {
    if (isBooleanType(node.type)) {
      return context.factory.createKeywordTypeNode(ts.SyntaxKind.BooleanKeyword);
    } else if (isBigintType(node.type)) {
      return context.factory.createKeywordTypeNode(ts.SyntaxKind.BigIntKeyword);
    }
  }

  return undefined;
}

/**
 * Check if the node is of bigint type.
 */
function isBigintType(v: ts.Node): boolean {
  if (ts.isLiteralTypeNode(v)) {
    const literalTypeNode = v as ts.LiteralTypeNode;
    if (literalTypeNode.literal && literalTypeNode.literal.kind !== undefined) {
      if (literalTypeNode.literal.kind === ts.SyntaxKind.PrefixUnaryExpression) {
        const prefix = literalTypeNode.literal as ts.PrefixUnaryExpression;
        return prefix.pos !== -1 && prefix.operand.kind === ts.SyntaxKind.BigIntLiteral;
      }
      return literalTypeNode.literal.kind === ts.SyntaxKind.BigIntLiteral;
    }
  }
  if (ts.isPrefixUnaryExpression(v) && v.operand.kind === ts.SyntaxKind.BigIntLiteral) {
    return true;
  }
  return v.kind === ts.SyntaxKind.BigIntKeyword || v.kind === ts.SyntaxKind.BigIntLiteral;
}

/**
 * Check if the node is of boolean type.
 */
function isBooleanType(v: ts.Node): boolean {
  if (ts.isLiteralTypeNode(v)) {
    const literalTypeNode = v as ts.LiteralTypeNode;
    if (literalTypeNode.literal && literalTypeNode.literal.kind !== undefined) {
      return (
        literalTypeNode.literal.kind === ts.SyntaxKind.FalseKeyword ||
        literalTypeNode.literal.kind === ts.SyntaxKind.TrueKeyword
      );
    }
  }
  return (
    v.kind === ts.SyntaxKind.FalseKeyword ||
    v.kind === ts.SyntaxKind.TrueKeyword ||
    v.kind === ts.SyntaxKind.BooleanKeyword
  );
}

function createTypeParameterDeclaration(
  node: ts.TypeParameterDeclaration,
  constraint: ts.TypeNode | undefined,
  defaultType: ts.TypeNode | undefined,
  context: ts.TransformationContext
): ts.TypeParameterDeclaration {
  return context.factory.createTypeParameterDeclaration(undefined, node.name, constraint, defaultType);
}

function extractTypeNodes(
  node: ts.TypeParameterDeclaration,
  context: ts.TransformationContext
): {
  unionTypeNode: ts.TypeNode | undefined;
  typeNode: ts.TypeNode | undefined;
} {
  let unionTypeNode: ts.TypeNode | undefined;
  let typeNode: ts.TypeNode | undefined;

  node.forEachChild((parm) => {
    if (!ts.isIdentifier(parm)) {
      if (ts.isUnionTypeNode(parm)) {
        unionTypeNode = createUnionTypeNode(parm, context) as ts.TypeNode;
      } else if (ts.isLiteralTypeNode(parm)) {
        typeNode = getLiteralType(parm, context);
      } else {
        typeNode = undefined;
      }
    }
  });

  return { unionTypeNode, typeNode };
}

function getLiteralType(parm: ts.LiteralTypeNode, context: ts.TransformationContext): ts.TypeNode | undefined {
  if (parm.literal.kind === ts.SyntaxKind.NumericLiteral) {
    return context.factory.createKeywordTypeNode(ts.SyntaxKind.NumberKeyword);
  } else if (parm.literal.kind === ts.SyntaxKind.TrueKeyword || parm.literal.kind === ts.SyntaxKind.FalseKeyword) {
    return context.factory.createKeywordTypeNode(ts.SyntaxKind.BooleanKeyword);
  }
  return undefined;
}

/**
 * Change the literal union type to a type union.
 */
function createUnionTypeNode(
  unionTypeNode: ts.UnionTypeNode,
  context: ts.TransformationContext
): ts.VisitResult<ts.Node> {
  const typeNode: ts.TypeNode[] = [];
  unionTypeNode.forEachChild((child) => {
    if (ts.isLiteralTypeNode(child)) {
      if (child.literal.kind === ts.SyntaxKind.FalseKeyword || child.literal.kind === ts.SyntaxKind.TrueKeyword) {
        typeNode.push(context.factory.createKeywordTypeNode(ts.SyntaxKind.BooleanKeyword));
      } else if (child.literal.kind === ts.SyntaxKind.NumericLiteral) {
        typeNode.push(context.factory.createKeywordTypeNode(ts.SyntaxKind.NumberKeyword));
      } else {
        typeNode.push(child);
      }
    } else {
      typeNode.push(child as ts.TypeNode);
    }
  });
  return context.factory.createUnionTypeNode(typeNode);
}

/**
 * Change the declarations of logical operators to the boolean type,
 * and change the declarations of bigint constants to the bigint type.
 */
function transformLogicalOperators(
  node: ts.Node,
  context: ts.TransformationContext,
  nodeFlag: ts.NodeFlags
): ts.VisitResult<ts.Node> {
  const variableDeclarations: ts.VariableDeclaration[] = [];
  node.forEachChild((child) => {
    if (ts.isVariableDeclaration(child)) {
      const processedDeclaration = processVariableDeclaration(child, nodeFlag, context);
      variableDeclarations.push(processedDeclaration);
    }
  });
  return context.factory.createVariableDeclarationList(variableDeclarations, nodeFlag);
}

/**
 * Process variable declarations
 */
function processVariableDeclaration(
  child: ts.VariableDeclaration,
  nodeFlag: ts.NodeFlags,
  context: ts.TransformationContext
): ts.VariableDeclaration {
  let updatedType: ts.TypeNode | undefined;
  child.forEachChild((v) => {
    updatedType = getUpdatedType(v, nodeFlag, context);
    return !!updatedType;
  });
  if (updatedType) {
    return context.factory.createVariableDeclaration(child.name, undefined, updatedType, undefined);
  }

  return child;
}

/**
 * Get the updated type.
 */
function getUpdatedType(
  v: ts.Node,
  nodeFlag: ts.NodeFlags,
  context: ts.TransformationContext
): ts.TypeNode | undefined {
  if (ts.isIdentifier(v)) {
    return undefined;
  }
  const isBigint = isBigintType(v);
  const isBoolean = isBooleanType(v);

  const allowedFlags = [ts.NodeFlags.Const, ts.NodeFlags.Let];
  if (isBigint && allowedFlags.includes(nodeFlag)) {
    return context.factory.createKeywordTypeNode(ts.SyntaxKind.BigIntKeyword);
  } else if (isBoolean) {
    return context.factory.createKeywordTypeNode(ts.SyntaxKind.BooleanKeyword);
  }
  return undefined;
}

function updatePropertyAccessExpression(
  node: ts.PropertyAccessExpression,
  context: ts.TransformationContext
): ts.TypeNode | undefined {
  let identifiers: ts.Identifier[] = [];
  if (ts.isPropertyAccessExpression(node.expression)) {
    identifiers = [node.expression.expression, node.expression.name] as ts.Identifier[];
  } else {
    identifiers = [node.expression] as ts.Identifier[];
  }
  if (identifiers.length > 1) {
    return context.factory.createTypeReferenceNode(context.factory.createQualifiedName(identifiers[0], identifiers[1]));
  } else if (identifiers.length === 1) {
    return context.factory.createTypeReferenceNode(identifiers[0]);
  }

  return undefined;
}

function filterImportDeclarationFromInvalidSDK(
  stmt: ts.ImportDeclaration,
  checker: ts.TypeChecker,
  current: ts.SourceFile,
  context: ts.TransformationContext
): ts.ImportDeclaration | undefined {
  if (isInvalidSDKDirectImport(stmt)) {
    return undefined;
  }

  const clause = stmt.importClause;
  if (!clause) {
    return stmt;
  }

  const updated = updateInvalidSDKImportClause(clause, checker, current, context);
  if (!updated) {
    return undefined;
  }

  return context.factory.updateImportDeclaration(
    stmt,
    stmt.modifiers,
    updated,
    stmt.moduleSpecifier,
    stmt.assertClause
  );
}

function isInvalidSDKDirectImport(stmt: ts.ImportDeclaration): boolean {
  const spec = stmt.moduleSpecifier;
  if (!ts.isStringLiteral(spec)) {
    return false;
  }
  return typeUtils.isIncompatibleSDK(spec.text);
}

function updateInvalidSDKImportClause(
  clause: ts.ImportClause,
  checker: ts.TypeChecker,
  current: ts.SourceFile,
  context: ts.TransformationContext
): ts.ImportClause | undefined {
  const newName = filterInvalidSDKDefaultImport(clause.name, checker, current);
  const newBindings = filterInvalidSDKNamedImports(clause.namedBindings, checker, current, context);

  if (!newName && !newBindings) {
    return undefined;
  }

  return context.factory.updateImportClause(clause, clause.isTypeOnly, newName, newBindings);
}

function filterInvalidSDKDefaultImport(
  name: ts.Identifier | undefined,
  checker: ts.TypeChecker,
  current: ts.SourceFile
): ts.Identifier | undefined {
  if (!name) {
    return name;
  }

  const imported = checker.getSymbolAtLocation(name);
  const real = imported ? checker.getAliasedSymbol(imported) : undefined;

  if (real && isSymbolFromInvalidSDK(real, current)) {
    return undefined;
  }

  return name;
}

function filterInvalidSDKNamedImports(
  bindings: ts.NamedImportBindings | undefined,
  checker: ts.TypeChecker,
  current: ts.SourceFile,
  context: ts.TransformationContext
): ts.NamedImportBindings | undefined {
  if (!bindings || !ts.isNamedImports(bindings)) {
    return bindings;
  }

  const valid: ts.ImportSpecifier[] = [];

  for (const element of bindings.elements) {
    const imported = checker.getSymbolAtLocation(element.name);
    const real = imported ? checker.getAliasedSymbol(imported) : undefined;

    if (!real || !isSymbolFromInvalidSDK(real, current)) {
      valid.push(element);
    }
  }

  if (valid.length === 0) {
    return undefined;
  }

  return context.factory.updateNamedImports(bindings, valid);
}

function filterExportDeclarationFromInvalidSDK(
  stmt: ts.ExportDeclaration,
  checker: ts.TypeChecker,
  current: ts.SourceFile,
  context: ts.TransformationContext
): ts.ExportDeclaration | undefined {
  if (isInvalidSDKReExport(stmt)) {
    return undefined;
  }

  const clause = stmt.exportClause;
  if (!clause || !ts.isNamedExports(clause)) {
    return stmt;
  }

  const valid = filterInvalidSDKExportSpecifiers(clause.elements, checker, current);
  if (valid.length === 0) {
    return undefined;
  }

  return context.factory.updateExportDeclaration(
    stmt,
    stmt.modifiers,
    stmt.isTypeOnly,
    context.factory.createNamedExports(valid),
    stmt.moduleSpecifier,
    stmt.assertClause
  );
}

function isInvalidSDKReExport(stmt: ts.ExportDeclaration): boolean {
  const spec = stmt.moduleSpecifier;
  if (!spec || !ts.isStringLiteral(spec)) {
    return false;
  }
  return typeUtils.isIncompatibleSDK(spec.text);
}

function filterInvalidSDKExportSpecifiers(
  elements: readonly ts.ExportSpecifier[],
  checker: ts.TypeChecker,
  current: ts.SourceFile
): ts.ExportSpecifier[] {
  const valid: ts.ExportSpecifier[] = [];

  for (const element of elements) {
    const exported = checker.getSymbolAtLocation(element.name);
    const real = exported ? checker.getAliasedSymbol(exported) : undefined;

    if (real && !isSymbolFromInvalidSDK(real, current)) {
      valid.push(element);
    }
  }

  return valid;
}

function filterExportAssignmentFromInvalidSDK(
  stmt: ts.ExportAssignment,
  typeChecker: ts.TypeChecker,
  currentSourceFile: ts.SourceFile
): ts.ExportAssignment | undefined {
  const symbol = typeChecker.getSymbolAtLocation(stmt.expression);
  if (symbol && isSymbolFromInvalidSDK(symbol, currentSourceFile)) {
    return undefined;
  }
  return stmt;
}

function isSymbolFromInvalidSDK(symbol: ts.Symbol, currentFile: ts.SourceFile): boolean {
  const declarations = symbol.getDeclarations();
  if (declarations) {
    for (const decl of declarations) {
      const sourceFileName = decl.getSourceFile().fileName;
      const sdkName = typeUtils.getBaseNameFromFilePath(sourceFileName);
      const currentFileName = typeUtils.getBaseNameFromFilePath(currentFile.fileName);
      if (typeUtils.isIncompatibleSDK(sdkName) && sdkName !== currentFileName) {
        return true;
      }
    }
  }
  return false;
}

function convertGenericTypeInLambdaIntoAny(
  node: ts.FunctionTypeNode,
  context: ts.TransformationContext
): ts.FunctionTypeNode {
  const typeParamNames = new Set(node.typeParameters?.map((tp) => tp.name.text) ?? []);

  const replaceTypeParams = (typeNode: ts.Node): ts.Node => {
    if (
      ts.isTypeReferenceNode(typeNode) &&
      ts.isIdentifier(typeNode.typeName) &&
      typeParamNames.has(typeNode.typeName.text)
    ) {
      return typeUtils.createJSValueTypeNode(context);
    }
    return ts.visitEachChild(typeNode, replaceTypeParams, context);
  };

  const newParameters = node.parameters.map((param) => {
    const newType = param.type ? (replaceTypeParams(param.type) as ts.TypeNode) : param.type;
    return context.factory.updateParameterDeclaration(
      param,
      param.modifiers,
      param.dotDotDotToken,
      param.name,
      param.questionToken,
      newType,
      param.initializer
    );
  });

  const newReturnType = node.type ? (replaceTypeParams(node.type) as ts.TypeNode) : node.type;

  return context.factory.updateFunctionTypeNode(
    node,
    undefined,
    ts.factory.createNodeArray(newParameters),
    newReturnType
  );
}

function convertOptionalMemberMethodToProperty(
  node: ts.MethodDeclaration | ts.MethodSignature,
  context: ts.TransformationContext
): ts.PropertyDeclaration | ts.PropertySignature {
  const functionType = context.factory.createFunctionTypeNode(
    node.typeParameters,
    node.parameters,
    node.type ?? typeUtils.createJSValueTypeNode(context)
  );

  if (node.typeParameters && node.typeParameters.length > 0) {
    convertGenericTypeInLambdaIntoAny(functionType, context);
  }

  if (ts.isMethodSignature(node)) {
    return context.factory.createPropertySignature(
      node.modifiers,
      node.name,
      context.factory.createToken(ts.SyntaxKind.QuestionToken),
      functionType
    );
  }

  if (ts.isMethodDeclaration(node)) {
    return context.factory.createPropertyDeclaration(
      node.modifiers,
      node.name,
      context.factory.createToken(ts.SyntaxKind.QuestionToken),
      functionType,
      undefined
    );
  }

  return node;
}

/**
 * helper function to filter out optional(with question token) methods in class declaration.
 */
function isNotOptionalMemberFunction(member: ts.ClassElement | ts.TypeElement): boolean {
  if (ts.isMethodDeclaration(member) && member.questionToken) {
    return false;
  } else if (ts.isMethodSignature(member) && member.questionToken) {
    return false;
  }
  return true;
}

function isReservedKeywordMember(member: ts.NamedDeclaration): boolean {
  const name = member.name;
  return !!name && ts.isIdentifier(name) && typeUtils.isHardOrReservedKeyword(name.text);
}

/**
 * Returns true when an enum has both numeric and string-valued members (heterogeneous enum).
 */
function isHeterogeneousEnum(node: ts.EnumDeclaration): boolean {
  let hasNumeric = false;
  let hasString = false;

  for (const member of node.members) {
    const init = member.initializer;
    if (!init) {
      continue;
    }
    if (ts.isNumericLiteral(init) || (ts.isPrefixUnaryExpression(init) && ts.isNumericLiteral(init.operand))) {
      hasNumeric = true;
    } else if (ts.isStringLiteral(init)) {
      hasString = true;
    }
    if (hasNumeric && hasString) {
      return true;
    }
  }

  return false;
}
