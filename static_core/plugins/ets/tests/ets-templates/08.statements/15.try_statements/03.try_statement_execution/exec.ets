{% for t in cases %}
/*---
desc: >-
  The try block and the entire try statement completes normally if catch blocks are not executed.
  Try block execution completes abruptly if an exception x is thrown inside the try block.
---*/

{{t.defs}}

function main(): int {
    {{t.use|indent}}
}
{% endfor %}
