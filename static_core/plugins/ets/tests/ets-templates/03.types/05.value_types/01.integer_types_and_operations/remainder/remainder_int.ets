{% for v in values %}
/*---
desc: check remainder of division operation for two integer operands
---*/

function main(): void {
    const a: int = {{v.left}}
    const b: int = {{v.right}}
    assert (a % b) == {{v.result}}
}

{% endfor %}
