#!/bin/sh

[ -z "$1" ] && exit 255

FN=$1.c
> $FN

echo "#define $1_main main" >> $FN
echo "#include \"$1_u.c\"" >> $FN

#echo -e "\nint main(int argc, char **argv){\n	return $1_main(argc, argv);\n}" >> $FN
