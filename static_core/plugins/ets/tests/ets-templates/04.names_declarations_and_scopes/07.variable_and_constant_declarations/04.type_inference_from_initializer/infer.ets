{% for c in cases %}

/*---
desc: Check type inference from the initialization expression
---*/

{{c.decl}}

function main(): void {
    {{c.use}}
    assert {{c.assert}}
}
{% endfor %}
