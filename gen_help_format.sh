FORMAT_FOLDER=help_format
IDIR=include

cp $FORMAT_FOLDER/help_template_head $IDIR/help.h

echo "#define ZFD_FORMAT \"zmidimap$(sed -n -e 'H;${x;s/\n/\\n/g;s/^,//;p;}' $FORMAT_FOLDER/zfd-format)\"" >> $IDIR/help.h
echo "#define MIM_FORMAT \"zmidimap$(sed -n -e 'H;${x;s/\n/\\n/g;s/^,//;p;}' $FORMAT_FOLDER/mim-format)\"" >> $IDIR/help.h
echo "#define SHELL_FORMAT \"zmidimap$(sed -n -e 'H;${x;s/\n/\\n/g;s/^,//;p;}' $FORMAT_FOLDER/shell-format)\"" >> $IDIR/help.h
echo "#define COMMAND_TAGS \"zmidimap$(sed -n -e 'H;${x;s/\n/\\n/g;s/^,//;p;}' $FORMAT_FOLDER/command-tags)\"" >> $IDIR/help.h

cat $FORMAT_FOLDER/help_template_tail >> $IDIR/help.h
