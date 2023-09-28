{% for c in cases %}

/*---
desc: A constant declaration introduces a named entity with mandatory initial value
---*/

{{c.decl}}

function main(): void {
    {{c.use}}
    assert {{c.assert}}
}
{% endfor %}
