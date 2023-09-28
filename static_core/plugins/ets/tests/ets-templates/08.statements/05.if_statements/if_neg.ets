{% for c in cases %}
/*---
desc: An if statement can be conditionally executed, or one of the two two statements can be selected conditionally.
tags: [compile-only, negative]
---*/

function main(): int {
    let i = 1;
    let j = 1;
    {{c.use|indent}}
    return 0;
}
{% endfor %}
