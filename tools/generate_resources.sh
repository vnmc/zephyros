#!/bin/bash

if [ $# -lt 2 ]
then
    echo "Syntax: generate_resources.sh <resource-asm-filename> <set-resource-id-include-filename>"
    exit
fi

ASM_FILE=$1
C_FILE=$2
tmp=`tempfile`

echo ".altmacro" > $ASM_FILE
echo ".macro binfile p q" >> $ASM_FILE
echo "    .globl \p&_begin" >> $ASM_FILE
echo "\p&_begin:" >> $ASM_FILE
echo "    .incbin \q" >> $ASM_FILE
echo "\p&_end:" >> $ASM_FILE
echo "    .globl \p&_len" >> $ASM_FILE
echo "\p&_len:" >> $ASM_FILE
echo ".int (\p&_end - \p&_begin)" >> $ASM_FILE
echo ".endm" >> $ASM_FILE
echo "" >> $ASM_FILE
echo ".data" >> $ASM_FILE

echo "#ifdef __cplusplus" > $C_FILE
echo "extern \"C\" {" >> $C_FILE
echo "#endif" >> $C_FILE
echo "#define BIN_DATA(_NAME) \\" >> $C_FILE
echo "    extern char _NAME##_begin; \\" >> $C_FILE
echo "    extern int _NAME##_len" >> $C_FILE
echo "#ifdef __cplusplus" >> $C_FILE
echo "}" >> $C_FILE
echo "#endif" >> $C_FILE
echo "" >> $C_FILE

i=0
pwd=`pwd`

# create the webapp directory name; Chromium lowercases the first part of the path ("domain");
# use sed to lowercase (\L) the characters until the first slash
webappdir=$(basename $pwd | sed 's/^[^\/]*/\L\0/')

for f in $(find . -type f)
do
    path=`readlink -f $f`
    echo "binfile __res$i \"$path\"" >> $ASM_FILE
    echo "BIN_DATA(__res$i);" >> $C_FILE
    echo "    Zephyros::SetResource(\"${path/$pwd/$webappdir}\", &__res${i}_begin, __res${i}_len);" >> $tmp
    ((i++))
done

echo "" >> $C_FILE
echo "void SetResources()" >> $C_FILE
echo "{" >> $C_FILE
cat $tmp >> $C_FILE
echo "}" >> $C_FILE
echo "" >> $C_FILE

rm $tmp
