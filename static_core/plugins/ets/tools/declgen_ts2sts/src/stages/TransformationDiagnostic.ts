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

export type DiagnosticSeverity = 'error' | 'warning' | 'info';

export type DiagnosticCategory =
  | 'unsupported-interop-scenario'
  | 'interop-tags'
  | 'unsafe-transform'
  | 'semantic-change'
  | 'internal';

export interface DiagnosticRange {
  fileName: string;
  start?: number;
  length?: number;
  line?: number;
  column?: number;
}

export interface DiagnosticRelatedInformation {
  message: string;
  range?: DiagnosticRange;
}

export interface TransformationDiagnostic {
  code: number;
  severity: DiagnosticSeverity;
  category: DiagnosticCategory;

  message: string;
  suggestion?: string;

  stage: string;

  range?: DiagnosticRange;
  related?: DiagnosticRelatedInformation[];

  nodeKind?: ts.SyntaxKind;
  autoFixable?: boolean;
  metadata?: Record<string, string | number | boolean | undefined>;
}
