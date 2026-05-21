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

import { DiagnosticCategory, DiagnosticSeverity, TransformationDiagnostic } from './TransformationDiagnostic';

/**
 * A callable that produces a partial TransformationDiagnostic (without `range` and `stage`,
 * which are filled in at the call site).
 */
type DiagnosticFactory<Args extends unknown[]> = (...args: Args) => Omit<TransformationDiagnostic, 'range' | 'stage'>;

/**
 * Define a typed diagnostic factory.
 *
 * Usage:
 *   const diag = DiagnosticMessages.interopTagUnknownSubcommand('foo');
 *   report({ ...diag, stage: this.name, range: rangeOf(node) });
 */
function define<Args extends unknown[]>(
  code: number,
  severity: DiagnosticSeverity,
  category: DiagnosticCategory,
  message: (...args: Args) => string
): DiagnosticFactory<Args> {
  return (...args: Args) => ({
    code,
    severity,
    category,
    message: message(...args)
  });
}

export const DiagnosticMessages = {
  // Interop tags =========================================================
  /** @interop / @noninterop used on a node kind that doesn't support it */
  interopTagUnsupportedNodeKind: define(
    116010,
    'error',
    'interop-tags',
    (tagName: string, nodeKind: string) => `@${tagName} is not supported on ${nodeKind}.`
  ),

  /** @interop has an unknown subcommand */
  interopTagUnknownSubcommand: define(
    116011,
    'error',
    'interop-tags',
    (subcommand: string) => `Unknown @interop subcommand "${subcommand}".`
  ),

  /** @interop param is missing the index argument */
  interopTagParamMissingIndex: define(
    116012,
    'error',
    'interop-tags',
    () => `@interop param requires an argument index, e.g. "@interop param 0 string".`
  ),

  /** @interop param index is not a valid non-negative integer */
  interopTagParamInvalidIndex: define(
    116013,
    'error',
    'interop-tags',
    (raw: string) => `@interop param index "${raw}" is not a valid non-negative integer.`
  ),

  /** @interop param/ret is missing the target type */
  interopTagMissingTargetType: define(
    116014,
    'error',
    'interop-tags',
    (subcommand: string) => `@interop ${subcommand} requires a target type, e.g. "@interop ${subcommand} string".`
  ),

  /** @noninterop has unexpected extra tokens */
  noninteropUnexpectedTokens: define(
    116015,
    'warning',
    'interop-tags',
    (extra: string) => `@noninterop does not take arguments; unexpected tokens: "${extra}".`
  ),

  /** @interop ret/param target type string is not a valid TypeScript type expression */
  interopTagInvalidTargetType: define(
    116016,
    'error',
    'interop-tags',
    (typeStr: string) => `@interop target type "${typeStr}" is not a valid type expression.`
  )
} as const;
