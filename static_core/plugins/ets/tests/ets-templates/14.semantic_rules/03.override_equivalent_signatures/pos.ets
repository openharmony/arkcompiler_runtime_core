{% for c in cases %}
/*---
desc: >-
    A compile-time error occurs if a package declares two functions with override-equivalent signatures, a class declares
    two methods with override-equivalent signatures or two constructors with override-equivalent signatures.
tags: {{c.tags}}
---*/

{{c.decl}}

function main(): int {
    return 0;
}
{% endfor %}
