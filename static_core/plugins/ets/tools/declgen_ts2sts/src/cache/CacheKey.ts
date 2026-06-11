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
import { canonicalJson, sha256 } from './Hash';
import { CACHE_FORMAT_VERSION, CACHE_LOGIC_VERSION, DECLGEN_IMPL_HASH } from './CacheVersion';

/**
 * Whitelist of compilerOptions fields that affect the semantics of generated
 * declarations. Path-only options that do not change emitted contents
 * (`outDir`, `tsBuildInfoFile`, `incremental`) are deliberately excluded.
 */
const RELEVANT_COMPILER_OPTION_KEYS: readonly (keyof ts.CompilerOptions)[] = [
  'target',
  'module',
  'moduleResolution',
  'lib',
  'strict',
  'strictNullChecks',
  'strictFunctionTypes',
  'strictBindCallApply',
  'strictPropertyInitialization',
  'noImplicitAny',
  'noImplicitThis',
  'noImplicitReturns',
  'alwaysStrict',
  'experimentalDecorators',
  'emitDecoratorMetadata',
  'jsx',
  'jsxFactory',
  'jsxFragmentFactory',
  'paths',
  'baseUrl',
  'typeRoots',
  'types',
  'allowJs',
  'checkJs',
  'esModuleInterop',
  'allowSyntheticDefaultImports',
  'isolatedModules',
  'useDefineForClassFields'
];

export interface DeclgenFeatures {
  enableInteropTypesFix: boolean;
  removeReservedKeywordIdentifier: boolean;
}

export interface PluginCacheKeyEntry {
  id: string;
  version: string;
  cacheVersion: string;
  config: unknown;
}

export interface GlobalKeyInput {
  declgenVersion: string;
  compilerOptions: ts.CompilerOptions;
  features: DeclgenFeatures;
  pipelineSignature: string;
  rootDir: string;
  outDir: string;
  /**
   * Per-plugin contribution. Any change (new plugin, removed plugin, version
   * bump, cacheVersion bump, config drift) invalidates the cache.
   */
  plugins?: readonly PluginCacheKeyEntry[];
}

export function pickRelevantCompilerOptions(opts: ts.CompilerOptions): Record<string, unknown> {
  const out: Record<string, unknown> = {};
  for (const key of RELEVANT_COMPILER_OPTION_KEYS) {
    const value = opts[key];
    if (value !== undefined) {
      out[key as string] = value as unknown;
    }
  }
  return out;
}

export function computeGlobalKey(input: GlobalKeyInput): string {
  const plugins = input.plugins
    ? [...input.plugins]
        .sort((a, b) => a.id.localeCompare(b.id))
        .map((p) => ({
          id: p.id,
          version: p.version,
          cacheVersion: p.cacheVersion,
          config: p.config ?? null
        }))
    : [];
  const payload = {
    cacheFormatVersion: CACHE_FORMAT_VERSION,
    cacheLogicVersion: CACHE_LOGIC_VERSION,
    declgenImplHash: DECLGEN_IMPL_HASH,
    declgenVersion: input.declgenVersion,
    tscVersion: ts.version,
    nodeMajor: process.versions.node.split('.')[0],
    compilerOptions: pickRelevantCompilerOptions(input.compilerOptions),
    features: input.features,
    pipelineSignature: input.pipelineSignature,
    rootDir: input.rootDir,
    outDir: input.outDir,
    plugins
  };
  return sha256(canonicalJson(payload));
}

export interface FileKeyInput {
  contentHash: string;
  depsSignature: string | null;
  resolvedImports: readonly string[];
}

export function computeFileKey(input: FileKeyInput): string {
  return sha256(
    canonicalJson({
      contentHash: input.contentHash,
      depsSignature: input.depsSignature,
      resolvedImports: [...input.resolvedImports].sort()
    })
  );
}
