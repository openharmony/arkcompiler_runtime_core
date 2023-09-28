{% for v in values %}
/*---
desc: check division for short integer operands
---*/

function main(): void {
    const a: short = {{v.left}}
    const b: short = {{v.right}}
    assert (a / b) == {{v.result}}
}

{% endfor %}
