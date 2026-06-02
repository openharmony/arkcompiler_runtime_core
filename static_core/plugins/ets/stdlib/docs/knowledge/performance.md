# Performance Knowledge

This document only records stdlib performance-related constraints and decision rules, with emphasis on how safepoint affects the choice of implementation approach. For RegExp native implementation, see `regexp.md`; for Intl native implementation, see `intl.md`.

## Core Premise: Safepoint and GC

Intrinsics (runtime built-in functions) **lack safepoints** internally and are not managed by the VM. When GC is triggered, it must wait for all threads to reach a safepoint, but intrinsic threads cannot respond in a timely manner, causing GC to block. **A safepoint interval exceeding 500μs can already have a significant impact on GC**.

This is the fundamental reason why stdlib **prohibits** heavy use of intrinsics, and it is also the key criterion for choosing between ANI and intrinsic.

## Implementation Choice

```
Need native implementation?
  │
  ├─ Involves third-party library calls (PCRE2, ICU4C, etc.)
  │    Execution time is uncontrollable, easily exceeds 500μs
  │    → **Must** use ANI native (as done for RegExp, Intl)
  │    → ANI is within VM management, safepoint is controllable
  │
  ├─ Pure logic, no third-party libraries involved
  │    │
  │    ├─ Extremely short (microsecond-level), with measured benefit
  │    │    → Intrinsic acceptable
  │    │    → But still need to confirm safepoint interval < 500μs
  │    │
  │    ├─ Heavy logic, may exceed 500μs
  │    │    → Use ANI native
  │    │    → Or manually insert safepoints in intrinsic
  │    │
  │    └─ ETS version can be inlined, fast enough
  │         → Do not promote to native
  │
  └─ irtoc
       → Only suitable for extremely short hot constructors/helper functions
       → Also subject to safepoint constraints
```

## Decision Boundaries

| Scenario | Approach | Reason |
| --- | --- | --- |
| Involves third-party libraries (PCRE2, ICU4C) | **Must** use ANI native | Execution time is uncontrollable, intrinsic will block GC |
| ETS version can be inlined, fast enough | Do not promote to native | Not necessary |
| Compiler or runtime fix can eliminate the performance need | Do not promote to native | Addressing the root cause is better than treating the symptom |
| Extremely short hot constructors/helper functions (< 500μs) | irtoc or intrinsic acceptable | Safepoint interval is within safe range |
| Heavy computation logic (>= 500μs) that must be intrinsic | Manually insert safepoints | Prevent GC blocking |
| Large-scale stdlib rewrite needs irtoc to be fast | Escalate as a compiler/runtime investigation | **Do not** migrate blindly |

## Constraints

- **Do not** promote an API to C++ intrinsic solely based on call frequency
- **Do not** promote an API to irtoc solely based on call frequency
- Native logic involving third-party library calls **must** be implemented through ANI; **intrinsic is prohibited**
- **No** continuous execution interval exceeding 500μs is allowed within an intrinsic; heavy logic **must** manually insert safepoints
- irtoc is **only suitable** for extremely small hot constructors/helper functions, or for low-level sequences with measurement proving that ETS/C++ cannot effectively lower them
- If intrinsic or fastpath work overflows beyond stdlib code, follow `../../../compiler/AGENTS.md`, `../../../runtime/AGENTS.md`, `../../../../../irtoc/AGENTS.md`

## Pre-Modification Checklist

- Is there performance data proving the need for native/irtoc?
- Is the ETS version already small enough to be inlined?
- Is there a compiler or runtime fix that could resolve the performance issue?
- Does it involve third-party library calls? If so, **must** use ANI rather than intrinsic
- Could the execution time of the intrinsic path exceed 500μs? If so, have safepoints been inserted?
- Does this need to be escalated as a compiler/runtime investigation?

## Code and Tests

- Compiler guidance: `../../../compiler/AGENTS.md`
- Runtime guidance: `../../../runtime/AGENTS.md`
- irtoc guidance: `../../../../../irtoc/AGENTS.md`
- ANI native implementation reference: `regexp.md`, `intl.md`
