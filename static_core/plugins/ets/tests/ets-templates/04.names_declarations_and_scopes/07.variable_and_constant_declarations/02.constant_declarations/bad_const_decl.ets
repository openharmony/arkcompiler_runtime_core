{% for c in cases %}
/*---
desc: Incorrect constant definition
tags: [negative, compile-only]
---*/

{{c.decl}}

function main(): void {
    {{c.use}}
}
{% endfor %}
