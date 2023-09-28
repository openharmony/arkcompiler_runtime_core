{% for t in cases %}
/*---
desc: >-
  A compile-time error occurs if:
    - the default catch clause is not the last catch clause in a try statement.
    - An exception parameter (if any) type reference is not the class Exception or a class derived from Exception or class Error or derived from it.
tags: [compile-only, negative]
---*/

{{t.defs}}

function main(): int {
    {{t.use|indent}}
}
{% endfor %}
