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
import { TransformationDiagnostic } from '../TransformationDiagnostic';
import { DiagnosticMessages } from '../TransformationDiagnosticMessages';

// ---------------------------------------------------------------------------
// Framework:
//
// To add a new @interop subcommand:
//   1. Add a new variant to InteropCommand.
//   2. Implement a SubcommandSpec and append it to INTEROP_SUBCOMMAND_SPECS.
//   3. Add allowed node kinds to COMMAND_ALLOWED_KINDS.
// ---------------------------------------------------------------------------

export type InteropCommand =
  | { type: 'noninterop' }
  | { type: 'interop'; subcommand: 'any' }
  | { type: 'interop'; subcommand: 'ret'; targetType: string }
  | { type: 'interop'; subcommand: 'param'; index: number; targetType: string }
  | { type: 'interop'; subcommand: 'break-extends' }
  | { type: 'interop'; subcommand: 'break-implements' };

/** Raw tokens extracted from a JSDoc tag before semantic validation. */
export interface InteropTokens {
  tagName: 'noninterop' | 'interop';
  tokens: string[];
}

export type ParseResult =
  | { ok: true; command: InteropCommand }
  | { ok: false; diagnostics: Array<Omit<TransformationDiagnostic, 'range' | 'stage'>> };

interface SubcommandSpec {
  /**
   * Return true when this spec should handle the given first token.
   * Runs in registration order; the first match wins.
   */
  matches: (firstToken: string) => boolean;

  /** Parse the full token list (tokens[0] is the subcommand token). */
  parse: (tokens: string[]) => ParseResult;
}

// ---------------------------------------------------------------------------
// Subcommand specs
// ---------------------------------------------------------------------------

const anySpec: SubcommandSpec = {
  matches: (t) => t === 'any',
  parse: (_tokens) => ({ ok: true, command: { type: 'interop', subcommand: 'any' } })
};

const retSpec: SubcommandSpec = {
  matches: (t) => t === 'ret',
  parse: (tokens) => {
    if (tokens.length < 2) {
      return { ok: false, diagnostics: [DiagnosticMessages.interopTagMissingTargetType('ret')] };
    }
    return { ok: true, command: { type: 'interop', subcommand: 'ret', targetType: tokens.slice(1).join(' ') } };
  }
};

const paramSpec: SubcommandSpec = {
  matches: (t) => t === 'param',
  parse: (tokens) => {
    // Syntax: @interop param <index> <targetType>
    // targetType may contain spaces: string | undefined, (A | B) | C, etc.
    if (tokens.length < 2) {
      return { ok: false, diagnostics: [DiagnosticMessages.interopTagParamMissingIndex()] };
    }
    const indexStr = tokens[1];
    const index = Number(indexStr);
    if (!Number.isInteger(index) || index < 0 || String(index) !== indexStr) {
      return { ok: false, diagnostics: [DiagnosticMessages.interopTagParamInvalidIndex(indexStr)] };
    }
    if (tokens.length < 3) {
      return { ok: false, diagnostics: [DiagnosticMessages.interopTagMissingTargetType('param')] };
    }
    return {
      ok: true,
      command: { type: 'interop', subcommand: 'param', index, targetType: tokens.slice(2).join(' ') }
    };
  }
};

const breakExtendsSpec: SubcommandSpec = {
  matches: (t) => t === 'break-extends',
  parse: (_tokens) => ({ ok: true, command: { type: 'interop', subcommand: 'break-extends' } })
};

const breakImplementsSpec: SubcommandSpec = {
  matches: (t) => t === 'break-implements',
  parse: (_tokens) => ({ ok: true, command: { type: 'interop', subcommand: 'break-implements' } })
};

/** Registry — append new SubcommandSpec here to extend the parser. */
const INTEROP_SUBCOMMAND_SPECS: SubcommandSpec[] = [anySpec, retSpec, paramSpec, breakExtendsSpec, breakImplementsSpec];

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

/** Parse a single InteropTokens into an InteropCommand or diagnostics. */
export function parseInteropCommand(interopTokens: InteropTokens): ParseResult {
  if (interopTokens.tagName === 'noninterop') {
    return parseNoninterop(interopTokens.tokens);
  }
  return parseInterop(interopTokens.tokens);
}

function parseNoninterop(tokens: string[]): ParseResult {
  if (tokens.length > 0) {
    return {
      ok: false,
      diagnostics: [DiagnosticMessages.noninteropUnexpectedTokens(tokens.join(' '))]
    };
  }
  return { ok: true, command: { type: 'noninterop' } };
}

function parseInterop(tokens: string[]): ParseResult {
  if (tokens.length === 0) {
    return {
      ok: false,
      diagnostics: [DiagnosticMessages.interopTagUnknownSubcommand('(empty)')]
    };
  }
  const firstToken = tokens[0];
  const spec = INTEROP_SUBCOMMAND_SPECS.find((s) => s.matches(firstToken));
  if (!spec) {
    return {
      ok: false,
      diagnostics: [DiagnosticMessages.interopTagUnknownSubcommand(firstToken)]
    };
  }
  return spec.parse(tokens);
}

// ---------------------------------------------------------------------------
// Node-kind validation
//
// To restrict a subcommand to specific node kinds, update the entry here.
// Use ts.SyntaxKind.Unknown as a placeholder for StructDeclaration if it is
// not present in the ambient typescript version.
// ---------------------------------------------------------------------------

type CommandKey =
  | Extract<InteropCommand, { type: 'noninterop' }>['type']
  | Extract<InteropCommand, { type: 'interop' }>['subcommand'];

function toKindSets<K extends string>(raw: Record<K, readonly ts.SyntaxKind[]>): Record<K, ReadonlySet<ts.SyntaxKind>> {
  return Object.fromEntries(
    Object.entries(raw).map(([k, v]) => [k, new Set(v as ts.SyntaxKind[])])
  ) as unknown as Record<K, ReadonlySet<ts.SyntaxKind>>;
}

export const COMMAND_ALLOWED_KINDS: Record<CommandKey, readonly ts.SyntaxKind[]> = {
  'noninterop': [
    ts.SyntaxKind.ClassDeclaration,
    ts.SyntaxKind.InterfaceDeclaration,
    ts.SyntaxKind.EnumDeclaration,
    ts.SyntaxKind.StructDeclaration,
    ts.SyntaxKind.VariableStatement,
    ts.SyntaxKind.FunctionDeclaration,
    ts.SyntaxKind.MethodDeclaration,
    ts.SyntaxKind.MethodSignature,
    ts.SyntaxKind.Constructor,
    ts.SyntaxKind.GetAccessor,
    ts.SyntaxKind.SetAccessor,
    ts.SyntaxKind.PropertyDeclaration,
    ts.SyntaxKind.PropertySignature,
    ts.SyntaxKind.EnumMember,
    ts.SyntaxKind.CallSignature,
    ts.SyntaxKind.ConstructSignature,
    ts.SyntaxKind.IndexSignature,
    ts.SyntaxKind.TypeAliasDeclaration
  ],
  'any': [ts.SyntaxKind.ClassDeclaration, ts.SyntaxKind.InterfaceDeclaration, ts.SyntaxKind.EnumDeclaration],
  'ret': [ts.SyntaxKind.MethodDeclaration, ts.SyntaxKind.MethodSignature, ts.SyntaxKind.FunctionDeclaration],
  'param': [
    ts.SyntaxKind.MethodDeclaration,
    ts.SyntaxKind.MethodSignature,
    ts.SyntaxKind.Constructor,
    ts.SyntaxKind.FunctionDeclaration
  ],
  'break-extends': [ts.SyntaxKind.ClassDeclaration, ts.SyntaxKind.InterfaceDeclaration],
  'break-implements': [ts.SyntaxKind.ClassDeclaration]
};

const COMMAND_ALLOWED_KINDS_SET = toKindSets<CommandKey>(COMMAND_ALLOWED_KINDS);

function commandKey(command: InteropCommand): CommandKey {
  return command.type === 'noninterop' ? 'noninterop' : command.subcommand;
}

/**
 * Validate that `command` may be applied to a node of `nodeKind`.
 * Returns `undefined` when valid, or a partial diagnostic when not.
 *
 * Usage at the call site:
 *   const err = validateCommandForNode(command, node.kind, tokens.tagName);
 *   if (err) { report({ ...err, stage: this.name, range: rangeOf(node) }); continue; }
 */
export function validateCommandForNode(
  command: InteropCommand,
  nodeKind: ts.SyntaxKind,
  tagName: 'interop' | 'noninterop'
): Omit<TransformationDiagnostic, 'range' | 'stage'> | undefined {
  const allowed = COMMAND_ALLOWED_KINDS_SET[commandKey(command)];
  if (!allowed.has(nodeKind)) {
    return DiagnosticMessages.interopTagUnsupportedNodeKind(tagName, ts.SyntaxKind[nodeKind]);
  }
  return undefined;
}
