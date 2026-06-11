# declgen Plugin System

A small extension layer that lets out-of-tree code add stages to the declgen
pipeline **without** breaking the incremental cache.

Every plugin:

- registers one or more stages at a symbolic **extension point**, never at a
  raw pipeline index;
- declares how its stage interacts with file state (`dependencyKind`), so the
  framework can automatically add the right barriers / narrowing stages;
- participates in the **global cache key** — adding, removing, version-bumping,
  or reconfiguring a plugin automatically invalidates the cache.

## Quick start

```ts
import {
  Declgen,
  Stage,
  StageContext,
  DeclgenPlugin,
  PluginStageSpec
} from '@panda/declgen';

class MyAuditStage extends Stage<unknown, unknown> {
  override readonly cacheVersion = '1';
  override get name(): string { return 'Plugin(com.acme.audit)'; }

  execute(ctx: StageContext<unknown>): StageContext<unknown> {
    ctx.transformTargetInfos.forEach((info, file) => {
      if (info.affected) {
        // ...inspect ctx.compiler.getSourceFile(file)...
      }
    });
    return ctx;
  }
}

const audit: DeclgenPlugin = {
  name: 'audit',
  stages: (): PluginStageSpec[] => [{
    id: 'com.acme.audit',         // reverse-DNS, [A-Za-z0-9._-]+
    version: '1.0.0',              // bump on any behaviour change
    stage: new MyAuditStage(),
    dependencyKind: 'local-readonly',
    anchor: 'after-ast-fix'
  }],
  cacheKeyContribution: () => ({ rules: ['no-any', 'prefer-const'] })
};

const declgen = new Declgen({
  inputFiles: [...],
  outDir: 'out',
  cacheDir: '.declgen-cache',
  incremental: true,
  plugins: [audit]
});
declgen.run();
declgen.emit();
```

## Extension points

| Anchor                | Inserts immediately after…                                                                |
| --------------------- | ----------------------------------------------------------------------------------------- |
| `after-declaration`   | `CacheSaveStage(declaration)`                                                             |
| `after-interop-tags`  | `AffectedAnalysisStage(interopTags)`                                                      |
| `after-conversion`    | `AffectedAnalysisStage(conversion)`                                                       |
| `after-semantic-fix`  | `AffectedAnalysisStage(semanticFix)`                                                      |
| `after-interop-types` | `AffectedAnalysisStage(interopTypes)` _(only present when `enableInteropTypesFix` is on)_ |
| `after-ast-fix`       | `AffectedAnalysisStage(astFix)`                                                           |

Use `position: 'before'` on a `PluginStageSpec` to insert just before the
anchor instead of after.

## `dependencyKind` — pick correctly

| Kind             | Reads other files? | Writes files? | Framework behaviour                                                                                                                                                                                                         |
| ---------------- | ------------------ | ------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `local-readonly` | no                 | no            | Stage runs on the current affected set. No follow-up stage inserted.                                                                                                                                                        |
| `local-write`    | no                 | yes (own AST) | An `AffectedAnalysisStage(plugin:<id>)` is auto-appended to narrow on output hash.                                                                                                                                          |
| `cross-read`     | yes                | maybe         | Auto-appended `AffectedAnalysisStage(plugin:<id>)`. The reverse-import closure keeps any importer of a changed file in the affected set, so cross-file signals reach this stage. Set `requiresFreshChecker: true` if you query the TypeChecker. |
| `global`         | yes (project-wide) | maybe         | The framework force-marks **every** root file as affected before the stage runs, so the stage always sees the full project. No narrowing afterwards.                                                                        |

> If you read other files via the TypeChecker, declare `cross-read` (or
> `global`). Declaring it `local-*` silently drops cross-file signal and will
> corrupt downstream caches.

## How the cache stays correct

The plugin id, version, the stage's `cacheVersion`, and any value returned by
`cacheKeyContribution()` are all folded into the **global cache key** via
[`computeGlobalKey`](../cache/CacheKey.ts). Any drift invalidates the entire
cache. You therefore never need to manually wipe `.declgen-cache/` after a
plugin upgrade.

Per-file outputs of a `local-write` / `cross-read` plugin stage are stored
under `manifest.files[absPath].stageOutputHashes['plugin:<id>']` and compared
on the next run by `AffectedAnalysisStage` — the same machinery the core
stages use.

Plugin stages may **not** persist text artifacts to the cache directory; only
the `declaration` stage does so. Calling `CacheSession.putStageArtifact` from
a plugin stage throws.

## Validation guarantees

`new Declgen(...)` (and `Pipeline.validate()` at the start of every `run()`)
rejects misconfigurations eagerly:

- duplicate plugin `id`s,
- ids that don't match `^[A-Za-z0-9._-]+$`,
- missing `version`,
- extension points that don't exist in the current pipeline (e.g.
  `after-interop-types` when `enableInteropTypesFix` is off),
- a `CacheLoadStage → RecheckStage` barrier broken by a plugin stage that
  forgot to declare `requiresFreshChecker`.

## Debugging tips

- `DECLGEN_NO_CACHE=1` — force a cold run; useful to confirm a plugin
  produces the same output without cache help.
- `DECLGEN_VERIFY_CACHE=1` (when wired in CI) — runs incremental + full and
  diffs outputs; catches missing `cross-read` declarations.
- Suffix your stage's `name` with `'Plugin(<id>)'` so it shows up clearly in
  `StageContext.stageTimings` and pipeline diagnostics.
- Bump `cacheVersion` (on the stage) **or** `version` (on the spec) whenever
  the stage's output contract changes. Both feed the cache key.
