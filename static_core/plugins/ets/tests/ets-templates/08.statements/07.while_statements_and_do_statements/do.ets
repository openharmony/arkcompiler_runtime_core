{% for c in cases %}
/*---
desc: >-
    While statements and do statements execute an expression and a statement repeatedly
    while the expression value is true. The key difference is that whileStatement
    evaluates and checks expression first while doStatement executes statement first.
---*/

function main(): int {
    let i = 1
    let j = 1
    {{c.use|indent}}
    return 1;
}
{% endfor %}
