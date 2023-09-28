{% for vd in var_decl %}

/*---
desc: A variable declaration introduces a named variable and optionally assigns it an initial value
---*/

{{vd.decl}}

function main(): void {
    {{vd.use}}
    assert {{vd.assert}}
}

{% endfor %}
