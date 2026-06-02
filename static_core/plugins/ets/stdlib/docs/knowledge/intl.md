# Intl Knowledge

This document only records the architectural constraints and implementation boundaries of the Intl module. For performance decision rules, see `performance.md`.

## Core Model

The ETS layer connects to the native layer via ANI, and the native layer wraps ICU4C. Both registration paths (runtime + ANI library) call `InitCoreIntl()`.

```
ETS Layer (Intl.ets)                        Native Layer (native/core/)
├ Infrastructure                             ├ IntlState (g_intlState, 5 caches)
│  Locale / LocaleMatch / LocaleHelper       │  mutex synchronization
├ Formatting classes                         ├ Sub-modules: parse args → check cache → create ICU object → call ICU → assemble result
│  NumberFormat / DateTimeFormat             └ ICU4C third-party library
│  RelativeTimeFormat / ListFormat             (numberformatter / smpdtfmt / coll / brkiter / ...)
│  DisplayNames                              String bridging (IntlCommon.cpp):
├ Rules and segmentation classes                ANI String ←→ std::string ←→ ICU UnicodeString
│  Collator / PluralRules / Segmenter
└ Each class declares native methods           Data flow:
   ETS parses options → pass self+args+cache key → Native checks cache → ICU formats → ANI assembles result
```

## File Locations

| Layer | Key Files | Responsibility |
| --- | --- | --- |
| ETS | `std/core/Intl.ets` | 11 Intl classes |
| Entry | `native/core/Intl.{h,cpp}` | `InitCoreIntl()`: create State + register all sub-modules |
| State | `native/core/IntlState.{h,cpp}` | Global `g_intlState`: holds 5 caches |
| Bridging | `native/core/IntlCommon.{h,cpp}` | ANI↔ICU string conversion |
| Sub-modules | `native/core/Intl{Name}.{h,cpp}` | One file per sub-module + corresponding Cache file |

## Caching Mechanism

All native caches are held by `g_intlState` (`unique_ptr`), synchronized via mutex. The Locale cache is in the ETS layer.

| Cache | Object | Upper Limit | Eviction | Key | Key Build Side |
| --- | --- | --- | --- | --- | --- |
| NumberFormat | `LocalizedNumberFormatter` + `LocalizedNumberRangeFormatter` | 512 | When reaching 512, delete 51 consecutive entries (10%) | 17 fields concatenated without delimiters (`TagString()`) | Native: `ParseOptions()` |
| DateTimeFormat | `icu::DateFormat` | 512 | Same as above | `locale;pattern;calendar;hourCycle;numberingSystem;timeZone;timeZoneName;formatMatcher` | ETS: `generateCacheKey()` |
| Collator | `icu::Collator` | 512 | Same as above | `lang;collation;caseFirst` | Native |
| RelativeTimeFormat | `icu::RelativeDateTimeFormatter` | **10** | When reaching 10, delete 2 entries | `localeName;styleStr` | Native |
| hourCycle | `std::string` | None | None | locale string | Native |
| Locale (ETS) | `ParsedLocaleData` | **1** | Overwrite | BCP47 tag | ETS |

Adding new NumberFormat options **must** synchronously update `ParsedOptions` + `TagString()`; adding new DateTimeFormat options **must** synchronously update the ETS-side `generateCacheKey()`.

## Constraints

- **Prohibited**: Directly manipulating ETS object fields in the native layer; must use ANI API — Reason: bypassing ANI causes type unsafety and GC issues
- **Prohibited**: Bypassing the cache to create ICU formatters directly — Reason: ICU object creation is expensive; cache hits avoid redundant creation
- `g_intlState` is globally unique; thread safety **must** be ensured via mutex — Reason: multi-threaded concurrent formatting is a common scenario
- New sub-modules **must** be registered in `InitCoreIntl()`; **do not** register separately — Reason: both registration paths (runtime + ANI library) depend on the unified entry point
- New native methods **must** synchronously update the ANI signature string — Reason: signature mismatch causes runtime crashes
- DateTimeFormat skeleton patterns are constructed by the ETS layer; **do not** assemble them in the native layer — Reason: the ETS layer owns the options context; the native layer should not have pattern concatenation logic

## Pre-Modification Checklist

- Has the ANI signature string been added for new native methods?
- Is a new cache class needed? If so, it must be registered in `IntlState`
- Has the new sub-module registration been added in `InitCoreIntl()`?
- Are ICU objects properly released?
- Can the cache key uniquely distinguish formatter configurations?
- Are both registration paths covered?

## Code and Tests

- ETS source: `std/core/Intl.ets`
- Native source: `native/core/Intl*.{h,cpp}`
- Test directory: `../tests/ets_func_tests/std/core/` (Intl*Test.ets, dateformat/)
