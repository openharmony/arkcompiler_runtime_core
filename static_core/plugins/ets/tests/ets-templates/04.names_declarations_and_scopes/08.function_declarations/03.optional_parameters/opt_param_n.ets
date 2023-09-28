{% for c in cases %}
/*---
desc: >-
    A compile-time error occurs in p?: T if T is not a reference type.
tags: [compile-only, negative]
---*/

{{c.decl}}

function main() {
    {{c.call|indent}}
}
{% endfor %}
