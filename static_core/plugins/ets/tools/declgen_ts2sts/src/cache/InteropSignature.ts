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

import { sha256 } from './Hash';

/**
 * Compute a conservative interop signature for a source file. We scan the raw
 * text for `@interop ...` and `@noninterop` JSDoc-style tags and hash the
 * normalized list. Any addition / removal / modification of an interop tag
 * anywhere in the file changes the signature.
 *
 * This intentionally does NOT depend on the parsed AST so it can be computed
 * cheaply per file without involving the type checker.
 */
export function computeInteropSignature(sourceText: string): string {
  const tokens: string[] = [];
  const re = /@(noninterop|interop)(?:\s+([^\r\n*]*))?/g;
  let m: RegExpExecArray | null;
  while ((m = re.exec(sourceText)) !== null) {
    const tag = m[1];
    const args = (m[2] ?? '').trim().replace(/\s+/g, ' ');
    tokens.push(args ? `${tag} ${args}` : tag);
  }
  if (tokens.length === 0) {
    return '';
  }
  tokens.sort();
  return sha256(tokens.join('\n'));
}
