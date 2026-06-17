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
import * as path from 'node:path';
import { sha256 } from './Hash';
import { CACHE_FORMAT_VERSION } from './CacheVersion';

export type CoreStageName = 'declaration' | 'interopTags' | 'conversion' | 'semanticFix' | 'interopTypes' | 'astFix';
/**
 * Plugin-defined stages get a `plugin:<id>` namespace so their per-file
 * output hashes can be persisted in the same manifest map without colliding
 * with core stages. See `src/plugin/Plugin.ts`.
 */
export type PluginStageName = `plugin:${string}`;
export type StageName = CoreStageName | PluginStageName;

/** Stages that persist a text artifact on disk. Other stages only record output hashes. */
export const STAGES_WITH_TEXT_ARTIFACT: ReadonlySet<StageName> = new Set<StageName>(['declaration']);

export const MANIFEST_FILE = 'manifest.json';
export const TSBUILDINFO_FILE = 'tsbuildinfo';
export const INTEROP_SIG_FILE = 'interop-sig.json';
export const STAGES_DIR = 'stages';

export interface FileMeta {
  fileKey: string;
  interopSigHash: string | null;
  /** sha256 of the file's own source text — independent of dependency state. */
  contentHash: string;
}

/**
 * Per-file record of the user-visible declaration output that the previous
 * run wrote (or was told it wrote, via the Phase-3 custom-emit hook).
 * Used by InitialAffectedAnalysisStage to detect outputs the user deleted
 * or modified between runs so the file can be re-emitted.
 */
export interface EmittedArtifactInfo {
  /** Absolute path the byte stream was written to. */
  path: string;
  /** sha256 of the final written content (post `'use static'` prefix). */
  sha256: string;
  /** fs.Stats.mtimeMs at write time. */
  mtime: number;
  /** fs.Stats.size at write time, in bytes. */
  size: number;
}

export interface CachedFileEntry {
  fileKey: string;
  interopSigHash: string | null;
  contentHash: string;
  /** stage → absolute path to artifact on disk. */
  stageArtifacts: Partial<Record<StageName, string>>;
  /** stage → last run's printed-output sha256. */
  stageOutputHashes: Partial<Record<StageName, string>>;
  /** Last emit's user-visible output, if recorded. */
  emittedArtifact?: EmittedArtifactInfo;
}

interface ManifestFileEntry {
  fileKey: string;
  interopSigHash: string | null;
  contentHash: string;
  stageArtifacts: Partial<Record<StageName, string>>; // relative paths inside cacheDir
  stageOutputHashes: Partial<Record<StageName, string>>;
  mtime: number;
  emittedArtifact?: EmittedArtifactInfo;
}

interface Manifest {
  cacheFormatVersion: number;
  globalKey: string;
  createdAt: string;
  tsBuildInfoPath: string;
  files: Record<string, ManifestFileEntry>;
}

interface NewArtifact {
  absSrc: string;
  stage: StageName;
  content: string;
  meta: FileMeta;
}

export class CacheSession {
  /** Files carried over unchanged from the previous manifest. */
  private readonly inherited: Map<string, ManifestFileEntry>;
  /** Per-file updates: meta to write under manifest.files[absSrc]. */
  private readonly updates: Map<string, ManifestFileEntry>;
  /** New artifact writes to be flushed on commit(). */
  private readonly pendingArtifacts: NewArtifact[] = [];
  /** Per-file output hashes recorded by AffectedAnalysisStage during this run. */
  private readonly pendingOutputHashes: Map<string, Partial<Record<StageName, string>>>;

  constructor(
    public readonly cacheDir: string,
    public readonly globalKey: string,
    inheritedFiles: Map<string, ManifestFileEntry>,
    public readonly tsBuildInfoPath: string
  ) {
    this.inherited = inheritedFiles;
    this.updates = new Map();
    this.pendingOutputHashes = new Map();
  }

  /** Look up a cached entry that may be reused, resolving relative paths to absolute. */
  getFile(absSrc: string): CachedFileEntry | undefined {
    const entry = this.inherited.get(absSrc);
    if (!entry) {
      return undefined;
    }
    const stageArtifacts: Partial<Record<StageName, string>> = {};
    for (const [stage, rel] of Object.entries(entry.stageArtifacts) as [StageName, string][]) {
      stageArtifacts[stage] = path.resolve(this.cacheDir, rel);
    }
    return {
      fileKey: entry.fileKey,
      interopSigHash: entry.interopSigHash,
      contentHash: entry.contentHash,
      stageArtifacts,
      stageOutputHashes: { ...entry.stageOutputHashes },
      emittedArtifact: entry.emittedArtifact ? { ...entry.emittedArtifact } : undefined
    };
  }

  /**
   * Record (or overwrite) the emitted-artifact metadata for a source file.
   * Called after the emit pipeline writes a user-visible declaration file so
   * the next run can detect external tampering or deletion.
   */
  recordEmittedArtifact(absSrc: string, info: EmittedArtifactInfo): void {
    let cur = this.updates.get(absSrc);
    if (!cur) {
      const inherited = this.inherited.get(absSrc);
      if (inherited) {
        cur = cloneEntry(inherited);
      } else {
        cur = {
          fileKey: '',
          interopSigHash: null,
          contentHash: '',
          stageArtifacts: {},
          stageOutputHashes: {},
          mtime: Date.now()
        };
      }
      this.updates.set(absSrc, cur);
    }
    cur.emittedArtifact = { ...info };
  }

  /**
   * Drop the recorded emitted-artifact metadata for a source file. Used when
   * the previously-emitted user file was deleted or modified between runs, so
   * the next emit treats this root as "never emitted" and rewrites it.
   */
  clearEmittedArtifact(absSrc: string): void {
    const inherited = this.inherited.get(absSrc);
    if (inherited) {
      inherited.emittedArtifact = undefined;
    }
    const cur = this.updates.get(absSrc);
    if (cur) {
      cur.emittedArtifact = undefined;
    }
  }

  /** Write a stage text artifact and record it in the upcoming manifest. */
  putStageArtifact(absSrc: string, stage: StageName, content: string, meta: FileMeta): void {
    if (!STAGES_WITH_TEXT_ARTIFACT.has(stage)) {
      throw new Error(
        `CacheSession.putStageArtifact: stage '${stage}' is not allowed to persist a text artifact. ` +
          `Plugin stages must record output hashes only.`
      );
    }
    const hash = sha256(content);
    const rel = path.join(STAGES_DIR, stage, hash.slice(0, 2), hash + artifactExtension(stage));
    this.pendingArtifacts.push({ absSrc, stage, content, meta });

    const cur = this.ensureUpdate(absSrc, meta);
    cur.stageArtifacts[stage] = rel;
    cur.fileKey = meta.fileKey;
    cur.interopSigHash = meta.interopSigHash;
    cur.contentHash = meta.contentHash;
  }

  /** Record a per-stage output hash for next-run AffectedAnalysisStage comparison. */
  putStageOutputHash(absSrc: string, stage: StageName, hash: string): void {
    let bucket = this.pendingOutputHashes.get(absSrc);
    if (!bucket) {
      bucket = {};
      this.pendingOutputHashes.set(absSrc, bucket);
    }
    bucket[stage] = hash;
  }

  /**
   * Drop any inherited record for this file so the next manifest does not
   * resurrect a stale entry. Call this when affected analysis decides the
   * file's source has been removed from the build inputs.
   */
  forget(absSrc: string): void {
    this.inherited.delete(absSrc);
    this.updates.delete(absSrc);
    this.pendingOutputHashes.delete(absSrc);
  }

  /** Atomic write: artifacts + manifest, then prune orphan artifacts. */
  async commit(): Promise<void> {
    fs.mkdirSync(this.cacheDir, { recursive: true });

    for (const a of this.pendingArtifacts) {
      const rel = this.updates.get(a.absSrc)?.stageArtifacts[a.stage];
      if (!rel) {
        continue;
      }
      const abs = path.resolve(this.cacheDir, rel);
      fs.mkdirSync(path.dirname(abs), { recursive: true });
      writeAtomic(abs, a.content);
    }

    const files: Record<string, ManifestFileEntry> = {};
    for (const [k, v] of this.inherited.entries()) {
      files[k] = cloneEntry(v);
    }
    for (const [k, v] of this.updates.entries()) {
      files[k] = cloneEntry(v);
    }
    for (const [k, hashes] of this.pendingOutputHashes.entries()) {
      const e = files[k] ?? this.ensureUpdate(k, { fileKey: '', interopSigHash: null, contentHash: '' });
      e.stageOutputHashes = { ...(e.stageOutputHashes ?? {}), ...hashes };
      files[k] = e;
    }

    const manifest: Manifest = {
      cacheFormatVersion: CACHE_FORMAT_VERSION,
      globalKey: this.globalKey,
      createdAt: new Date().toISOString(),
      tsBuildInfoPath: path.relative(this.cacheDir, this.tsBuildInfoPath) || TSBUILDINFO_FILE,
      files
    };
    writeAtomic(path.join(this.cacheDir, MANIFEST_FILE), JSON.stringify(manifest, null, 2));

    pruneOrphanArtifacts(this.cacheDir, files);
  }

  private ensureUpdate(absSrc: string, meta: FileMeta): ManifestFileEntry {
    let cur = this.updates.get(absSrc);
    if (!cur) {
      const inherited = this.inherited.get(absSrc);
      cur = {
        fileKey: meta.fileKey,
        interopSigHash: meta.interopSigHash,
        contentHash: meta.contentHash,
        stageArtifacts: inherited ? { ...inherited.stageArtifacts } : {},
        stageOutputHashes: inherited ? { ...inherited.stageOutputHashes } : {},
        mtime: Date.now(),
        // Carry forward the inherited emittedArtifact so that a stage-artifact
        // write does not silently wipe the previous run's emit record on commit.
        // The prior emit fingerprint must be preserved because a file may be
        // reprocessed by the pipeline (via ensureUpdate) yet produce byte-identical
        // output, in which case it is NOT re-emitted and recordEmittedArtifact is
        // never called. Without preserving it here, the inherited emittedArtifact
        // would be dropped on commit and the next run would lose its baseline,
        // forcing a redundant re-emit.
        emittedArtifact: inherited?.emittedArtifact ? { ...inherited.emittedArtifact } : undefined
      };
      this.updates.set(absSrc, cur);
    } else {
      cur.fileKey = meta.fileKey;
      cur.interopSigHash = meta.interopSigHash;
      cur.contentHash = meta.contentHash;
      cur.mtime = Date.now();
    }
    return cur;
  }
}

function cloneEntry(e: ManifestFileEntry): ManifestFileEntry {
  return {
    fileKey: e.fileKey,
    interopSigHash: e.interopSigHash,
    contentHash: e.contentHash,
    stageArtifacts: { ...e.stageArtifacts },
    stageOutputHashes: { ...e.stageOutputHashes },
    mtime: e.mtime,
    emittedArtifact: e.emittedArtifact ? { ...e.emittedArtifact } : undefined
  };
}

function artifactExtension(stage: StageName): string {
  switch (stage) {
    case 'declaration':
      return '.dets';
    default:
      return '.txt';
  }
}

function writeAtomic(filePath: string, content: string): void {
  const tmp = filePath + '.tmp';
  fs.writeFileSync(tmp, content, { encoding: 'utf8' });
  fs.renameSync(tmp, filePath);
}

function pruneOrphanArtifacts(cacheDir: string, files: Record<string, ManifestFileEntry>): void {
  const referenced = new Set<string>();
  for (const f of Object.values(files)) {
    for (const rel of Object.values(f.stageArtifacts)) {
      if (rel) {
        referenced.add(path.resolve(cacheDir, rel));
      }
    }
  }
  const stagesDir = path.join(cacheDir, STAGES_DIR);
  if (!fs.existsSync(stagesDir)) {
    return;
  }
  walkAndDeleteOrphans(stagesDir, referenced);
}

function walkAndDeleteOrphans(dir: string, referenced: Set<string>): void {
  for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
    const p = path.join(dir, entry.name);
    if (entry.isDirectory()) {
      walkAndDeleteOrphans(p, referenced);
      try {
        if (fs.readdirSync(p).length === 0) {
          fs.rmdirSync(p);
        }
      } catch {
        // ignore
      }
    } else if (entry.isFile() && !referenced.has(p)) {
      try {
        fs.unlinkSync(p);
      } catch {
        // ignore
      }
    }
  }
}

export interface OpenOptions {
  cacheDir: string;
  globalKey: string;
}

export interface CacheStore {
  open(options: OpenOptions): CacheSession;
}

export const cacheStore: CacheStore = {
  open(options: OpenOptions): CacheSession {
    const { cacheDir, globalKey } = options;
    fs.mkdirSync(cacheDir, { recursive: true });
    const manifestPath = path.join(cacheDir, MANIFEST_FILE);
    const tsBuildInfoPath = path.join(cacheDir, TSBUILDINFO_FILE);

    const inherited = loadInheritedFiles(manifestPath, globalKey);
    if (!inherited) {
      wipeCacheDir(cacheDir);
    }

    return new CacheSession(cacheDir, globalKey, inherited ?? new Map(), tsBuildInfoPath);
  }
};

/**
 * Tries to load the manifest at `manifestPath` and deserialise its file
 * entries. Returns the inherited map on success, or `undefined` when the
 * manifest is absent, unreadable, or does not match the expected global key /
 * format version (any of which means the cache must be wiped).
 */
function loadInheritedFiles(
  manifestPath: string,
  globalKey: string
): Map<string, ManifestFileEntry> | undefined {
  if (!fs.existsSync(manifestPath)) {
    return undefined;
  }
  try {
    const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8')) as Manifest;
    if (manifest.cacheFormatVersion !== CACHE_FORMAT_VERSION || manifest.globalKey !== globalKey || !manifest.files) {
      return undefined;
    }
    const inherited = new Map<string, ManifestFileEntry>();
    for (const [k, v] of Object.entries(manifest.files)) {
      inherited.set(k, {
        fileKey: v.fileKey,
        interopSigHash: v.interopSigHash ?? null,
        contentHash: v.contentHash ?? '',
        stageArtifacts: v.stageArtifacts ?? {},
        stageOutputHashes: v.stageOutputHashes ?? {},
        mtime: v.mtime ?? 0,
        emittedArtifact: v.emittedArtifact
      });
    }
    return inherited;
  } catch {
    return undefined;
  }
}

function wipeCacheDir(cacheDir: string): void {
  for (const entry of fs.readdirSync(cacheDir, { withFileTypes: true })) {
    const p = path.join(cacheDir, entry.name);
    fs.rmSync(p, { recursive: true, force: true });
  }
}
