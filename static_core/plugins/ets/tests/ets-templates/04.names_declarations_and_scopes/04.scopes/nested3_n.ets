/*---
desc: >-
  Scopes of two names can overlap (e.g., when statements are nested),
  in this case access to the outer name is not accessible because the
  name with the innermost declaration takes precedence.
tags: [compile-only, negative]
---*/

function main() {
    if (true) {
        let x = 10
    }

    assert x == 10  // CTE
}
