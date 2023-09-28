{% for c in cases %}
/*---
desc: >-
  A compound assignment expression of the form E1 op= E2 is equivalent to E1 = ((E1) op (E2)) as T,
  where T is the type of E1, except that E1 is evaluated only once.
params: {{c.type}}, "{{c.op|safe}}="
tags: [negative]
---*/

function main(): int {
    let v:{{c.type}} = {{c.v}}
    let q:{{c.type}} = {{c.q}}
    if ((v {{c.op}} q) != (v {{c.op}}= q)) {
        return 1
    }
    return 0
}
{% endfor %}
