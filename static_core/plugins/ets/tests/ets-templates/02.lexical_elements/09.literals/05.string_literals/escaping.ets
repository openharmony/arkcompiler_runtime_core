/*---
desc: An escape sequence always starts with the backslash character \
---*/

function main() {
    let s1 = '\"\'\b\f\n\r\t\v\\\A\B\C\d\+\=\-\(\)\#\$\@\!\~\`\<\>\/\?\*\{\}\[\]\|\;\:\%\^\&\ю\Я\ '  // single-quoted
    let s2 = "\"\'\b\f\n\r\t\v\\\A\B\C\d\+\=\-\(\)\#\$\@\!\~\`\<\>\/\?\*\{\}\[\]\|\;\:\%\^\&\ю\Я\ "  // double-quoted
    let q1 = '\u0022\u0027\u0008\u000c\u000a\u000d\u0009\u000b\u005c\A\B\C\d\+\=\-\(\)\#\$\@\!\~\`\<\>\/\?\*\{\}\[\]\|\;\:\%\^\&\ю\Я\ ' // single-quoted
    let q2 = "\u0022\u0027\u0008\u000c\u000a\u000d\u0009\u000b\u005c\A\B\C\d\+\=\-\(\)\#\$\@\!\~\`\<\>\/\?\*\{\}\[\]\|\;\:\%\^\&\ю\Я\ " // double-quoted
    assert s1 == s2 && s1 == q1 && s1 == q2
}
