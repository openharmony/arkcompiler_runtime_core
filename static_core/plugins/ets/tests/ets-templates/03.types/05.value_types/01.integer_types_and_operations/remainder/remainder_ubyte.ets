{% for v in values %}
/*---
desc: check remainder of division operation for two unsigned byte operands
---*/

function main(): void {
    const a: ubyte = {{v.left}}
    const b: ubyte = {{v.right}}
    assert (a % b) == {{v.result}}
}

{% endfor %}
