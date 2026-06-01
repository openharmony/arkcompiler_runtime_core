# VerifyANI Error Reporting Knowledge

This file only covers VerifyANI invalid-usage reporting, abort/error hooks, and return-status boundaries. For checker selection, read `verifyani_checker_rules.md`.

## Core Model

| Mechanism | Code anchor | Role |
| --- | --- | --- |
| `ANIVerifier::Report` | `../../ani_verifier.*` | Central invalid-usage reporting entry |
| Abort hook | `ANIVerifier::SetAbortHook` | Optional fatal reporting hook |
| Error hook | `ANIVerifier::SetErrorHook` | Optional non-fatal reporting hook |
| Wrapper report path | `../../verify_ani_interaction_api.cpp` | Converts validation failure to ANI status |
| Workaround flag | `ANIVerifier::SetVerifyOptions`, private `IsWorkaroundNoCrashIfInvalidUsage` | Internal policy for invalid usage handling |

## Constraints

- Keep verifier reporting behavior deterministic for a given invalid input.
- Do not expose `IsWorkaroundNoCrashIfInvalidUsage` outside `ANIVerifier`.
- Do not delete report calls to make a negative test pass.
- Prefer nearby tests as evidence for expected report behavior; if message text is not covered by stable tests, mark it as uncertain in analysis/final response.

## Return Status Rules

| Situation | Expected pattern |
| --- | --- |
| Wrapper argument verification fails | Report invalid usage and return `ANI_ERROR` unless existing API-specific code uses another status |
| Release API returns non-OK | Preserve release status unless wrapper has documented extra validation |
| Output pointer invalid | Return invalid-args style status only if the existing checker/API pattern does so |
| Invalid ref/method/field wrapper | Treat as VerifyANI invalid usage, not as a release object lookup miss |

## Modification Checklist

| Question | Required check |
| --- | --- |
| Does this change fatal vs non-fatal reporting? | Inspect `ANIVerifier::Report`, hooks, and workaround flag |
| Does this change message text? | Run focused VerifyANI negative test or state missing coverage |
| Does release ANI behavior also change? | Run matching release ANI test |

## Code And Tests

| Area | Code | Tests |
| --- | --- | --- |
| Report hooks/policy | `../../ani_verifier.*` | `../../../../../tests/ani/tests/verifyani/` focused negative tests |
| Wrapper failure paths | `../../verify_ani_interaction_api.cpp`, `../../verify_ani_vm_api.cpp` | Matching API family under `../../../../../tests/ani/tests/verifyani/` |
| Checker messages | `../../verify_ani_checker.cpp` | Nearby `verify_*_test.cpp` or generated `.cpp.j2` |
