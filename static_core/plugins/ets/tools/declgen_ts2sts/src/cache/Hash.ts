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

import * as crypto from 'node:crypto';

/** sha256 hex digest of the given string/buffer. */
export function sha256(input: string | Buffer): string {
  return crypto.createHash('sha256').update(input).digest('hex');
}

/**
 * Stable JSON serialization: object keys are recursively sorted so the output
 * only depends on values, not insertion order. Used to feed sha256 cache keys.
 */
export function canonicalJson(value: unknown): string {
  if (value === null || value === undefined) {
    return JSON.stringify(value === undefined ? null : value);
  }
  if (typeof value !== 'object') {
    return JSON.stringify(value);
  }
  if (Array.isArray(value)) {
    return '[' + value.map(canonicalJson).join(',') + ']';
  }
  const obj = value as Record<string, unknown>;
  const keys = Object.keys(obj).sort();
  const body = keys.map((k) => JSON.stringify(k) + ':' + canonicalJson(obj[k])).join(',');
  return '{' + body + '}';
}
