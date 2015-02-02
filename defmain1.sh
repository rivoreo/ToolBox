#!/bin/sh

FN=$1.c
> $FN

echo "#define $1_main main" >> $FN
echo "#include \"$1_u.c\"" >> $FN

#echo "\nint main(int argc, char **argv){\n	return $1_main(argc, argv);\n}" >> $FN
