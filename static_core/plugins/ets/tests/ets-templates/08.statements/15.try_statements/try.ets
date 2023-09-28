{% for t in cases %}
/*---
desc: >-
  Try statements run the block of code to handle exceptions and errors.
  Try statements can contain several catch clauses to handle exceptions and
  errors.
---*/

class Ex extends Exception {}
class F extends Exception {}
class X extends Error {}
class Y extends Error {}

let t1 = 1, t2 = 2  // global vars, never changed in tests

{{t.defs}}
function main(): int {
    {{t.use|indent}}
}
{% endfor %}
