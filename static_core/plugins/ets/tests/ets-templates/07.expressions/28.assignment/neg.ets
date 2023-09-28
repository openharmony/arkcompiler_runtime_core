{% for c in cases %}
/*---
desc: The result of the first operand of an assignment operator must be a variable, or a compile-time error occurs.
tags: [compile-only, negative]
---*/

function main(): int {
    {{c.expr|indent}}
    return 0
}
{% endfor %}
