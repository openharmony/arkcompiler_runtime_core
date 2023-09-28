{% for t in cases %}
/*---
desc: >-
    A finally clause defines a set of actions as a block which is
    executed regardless how try-catch was fulfilled normally or abruptly.
tags: [compile-only, negative]
---*/

class Ex extends Exception {}
class Ex2 extends Ex {}

class Er extends Error {}
class Er2 extends Er {}

let t1 = 1, t2 = 2  // global vars, never changed in tests

{{t.defs}}

function main(): int {
    {{t.use|indent}}
}
{% endfor %}
