{% for c in cases %}
/*---
desc: >-
    All assignment operators are syntactically right-associative (they group right-to-left).
    Thus, a=b=c means a=(b=c), which assigns the value of c to b and then assigns the value of b to a.
---*/
{{c.defs}}
function main(): int {
    {{c.expr|indent}}
    return 0
}
{% endfor %}
