{% for c in cases %}
/*---
desc: A smaller type's value can be assigned to a larger type's variable.
params: {{c.sm.split(" ").0}} to {{c.lg.split(" ").0}}
---*/

function main(): void {
    let smaller: {{c.sm}}
    {%- for op in  c['ops'] %}
    {
        let larger: {{c.lg}}
        larger {{op.op}} smaller
        assert larger == {{op.val}}
    }
    {%- endfor %}
}
{% endfor %}
