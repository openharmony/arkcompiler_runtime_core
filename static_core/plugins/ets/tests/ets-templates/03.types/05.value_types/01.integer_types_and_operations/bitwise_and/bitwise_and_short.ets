{% for v in values %}
/*---
desc: check bitwise AND operation for short operands
---*/

function main(): void {
    const a: short = {{v.left}}
    const b: short = {{v.right}}
    assert (a & b) == {{v.result}}
}

{% endfor %}
