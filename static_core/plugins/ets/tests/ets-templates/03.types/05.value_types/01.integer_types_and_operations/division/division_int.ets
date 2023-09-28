{% for v in values %}
/*---
desc: check division for integer operands
---*/

function main(): void {
    const a: int = {{v.left}}
    const b: int = {{v.right}}
    assert (a / b) == {{v.result}}
}

{% endfor %}
