{% for c in cases %}

/*---
desc: 'malformed parameterized generics: use of generics with non generic'
tags: [compile-only, negative]
---*/

{{c.decl}}

function main(): void {
    {{c.use}}
}

{% endfor %}
