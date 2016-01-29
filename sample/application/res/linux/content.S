.altmacro
.macro binfile p q
    .globl \p&_begin
\p&_begin:
    .incbin \q
\p&_end:
    .globl \p&_len
\p&_len:
.int (\p&_end - \p&_begin)
.endm

.data
binfile __res0 "/home/administrator/Projects/zephyros/sample/application/app/style.css"
binfile __res1 "/home/administrator/Projects/zephyros/sample/application/app/index.html"
binfile __res2 "/home/administrator/Projects/zephyros/sample/application/app/main.js"
binfile __res3 "/home/administrator/Projects/zephyros/sample/application/app/zepto.js"
