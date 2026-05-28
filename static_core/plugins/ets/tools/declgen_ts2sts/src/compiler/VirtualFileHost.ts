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

import * as fs from 'node:fs';
import path from 'node:path';
import * as ts from 'typescript';
import { Extension } from '../utils/Extension';

const TSBUILDINFO_EXT = '.tsbuildinfo';

export interface FileMeta {
  filePath: string;
  noChange: boolean;
  noWrite: boolean;
}

export interface File {
  meta: FileMeta;
  content: string;
}

export const INTERNAL_PREFIX = '?internal';

/**
 * Normalize an input path to an absolute path using forward slashes.
 *
 * TypeScript stores all `SourceFile.fileName` values with forward slashes
 * regardless of platform. Node's `path.resolve` on Windows uses backslashes,
 * which would cause map / set lookups keyed by `normalizePath(...)` to miss
 * when compared against a `SourceFile.fileName`. We normalize here so every
 * caller that funnels paths through `normalizePath` ends up with the same
 * representation that TypeScript uses internally.
 */
export function normalizePath(fileName: string): string {
  if (fileName.startsWith(INTERNAL_PREFIX)) {
    return fileName;
  }
  return path.resolve(fileName).replace(/\\/g, '/');
}

export function convertToDetsFileName(fileName: string): string {
  if (fileName.endsWith(Extension.DETS)) {
    return fileName;
  } else if (fileName.endsWith(Extension.DTS)) {
    return fileName.slice(0, -Extension.DTS.length) + Extension.DETS;
  } else if (fileName.endsWith(Extension.ETS)) {
    return fileName.slice(0, -Extension.ETS.length) + Extension.DETS;
  } else if (fileName.endsWith(Extension.TS)) {
    return fileName.slice(0, -Extension.TS.length) + Extension.DETS;
  }
  return path.basename(fileName) + Extension.DETS;
}

export class VirtualFileHost {
  private files: Map<string, File>;
  private baseHost: ts.CompilerHost;
  private compilerOptions: ts.CompilerOptions;
  private sourceFileCache: Map<string, { content: string; sourceFile: ts.SourceFile }> = new Map();

  constructor(compilerOptions: ts.CompilerOptions) {
    this.files = new Map<string, File>();
    this.compilerOptions = compilerOptions;
    this.baseHost = ts.createCompilerHost(this.compilerOptions);
  }

  /**
   * Caches a file by content in the virtual file system.
   * @param content The content of the file to cache.
   * @param meta The metadata associated with the file.
   */
  cache(content: string, meta: FileMeta): void {
    const filePath = normalizePath(meta.filePath);
    this.files.set(filePath, {
      meta,
      content
    });
  }

  private isCacheableFile(fileName: string): boolean {
    const cacheableExtensions: readonly string[] = [Extension.DETS, Extension.DTS, Extension.ETS, Extension.TS];
    return cacheableExtensions.some((ext) => fileName.endsWith(ext));
  }

  /**
   * Reads the content of a file.
   * First checks the virtual file system, if the file is not found,
   * it falls back to the base host's readFile method.
   * @param fileName The name of the file to read.
   * @returns The content of the file, or undefined if the file does not exist.
   */
  readFile(fileName: string): string | undefined {
    fileName = normalizePath(fileName);
    const file = this.files.get(fileName);
    if (!file) {
      const content = this.baseHost.readFile(fileName);
      if (content !== undefined && this.isCacheableFile(fileName)) {
        this.files.set(fileName, {
          meta: {
            filePath: fileName,
            noChange: false,
            noWrite: false
          },
          content
        });
      }
      return content;
    }
    return file.content;
  }

  /**
   * Updates the content of a file in the virtual file system.
   * If the file does not exist, it will be created.
   * @param fileName The name of the file to update.
   * @param content The new content of the file.
   */
  updateFile(fileName: string, content: string): void {
    fileName = normalizePath(fileName);
    const file = this.files.get(fileName);
    if (file) {
      if (file.meta.noChange) {
        return;
      }
      file.content = content;
    } else {
      this.files.set(fileName, {
        meta: {
          filePath: fileName,
          noChange: false,
          noWrite: false
        },
        content
      });
    }
  }

  /**
   * Checks if a file exists in the virtual file system or the base host.
   * @param fileName The name of the file to check.
   * @returns True if the file exists, false otherwise.
   */
  fileExists(fileName: string): boolean {
    fileName = normalizePath(fileName);
    return this.files.has(fileName) || this.baseHost.fileExists(fileName);
  }

  /**
   * Creates a hash of the given data using the base host's createHash method.
   * @param data The data to hash.
   * @returns The hash of the data.
   */
  createHash(data: string): string {
    return ts.sys.createHash!(data);
  }

  getCompilerHost(): ts.CompilerHost {
    const getOrCreateSourceFile = (
      fileName: string,
      languageVersion: ts.ScriptTarget | ts.CreateSourceFileOptions
    ): ts.SourceFile | undefined => {
      const content = this.readFile(fileName);
      if (content === undefined) {
        return undefined;
      }
      const cached = this.sourceFileCache.get(fileName);
      if (cached && cached.content === content) {
        return cached.sourceFile;
      }
      const sf = ts.createSourceFile(fileName, content, languageVersion);
      // Builder programs (incremental mode) require every SourceFile to
      // carry a `version`. ts.createSourceFile does not set one, so we
      // derive it from the content hash here.
      (sf as ts.SourceFile & { version: string }).version = this.createHash(content);
      this.sourceFileCache.set(fileName, { content, sourceFile: sf });
      return sf;
    };

    const writeBuildInfo = (fileName: string, text: string, onError?: (message: string) => void): void => {
      try {
        const dir = path.dirname(fileName);
        if (dir && !fs.existsSync(dir)) {
          fs.mkdirSync(dir, { recursive: true });
        }
        fs.writeFileSync(fileName, text, { encoding: 'utf8' });
      } catch (error) {
        onError?.(String(error));
      }
    };

    const host: ts.CompilerHost = {
      ...this.baseHost,
      getSourceFile: (fileName, languageVersion, onError) => {
        fileName = normalizePath(fileName);
        return (
          getOrCreateSourceFile(fileName, languageVersion) ??
          this.baseHost.getSourceFile(fileName, languageVersion, onError)
        );
      },
      readFile: (fileName) => this.readFile(fileName),
      fileExists: (fileName) => this.fileExists(fileName),
      writeFile: (fileName, text, _writeByteOrderMark, onError) => {
        // Allow tsc to persist its incremental build info on disk so that
        // subsequent runs can reuse parse / type-check results. The configured
        // `compilerOptions.tsBuildInfoFile` may use any basename (e.g. the
        // declgen cache layout writes it as `<cacheDir>/tsbuildinfo` with no
        // extension), so accept it by path equality in addition to the default
        // `.tsbuildinfo` extension probe.
        const configured = this.compilerOptions.tsBuildInfoFile;
        if (fileName.endsWith(TSBUILDINFO_EXT) || (configured && path.resolve(fileName) === path.resolve(configured))) {
          writeBuildInfo(fileName, text, onError);
        }
        return null;
      },
      createHash: (data) => this.createHash(data)
    };
    return host;
  }
}
