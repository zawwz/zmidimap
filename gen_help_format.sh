FORMAT_FOLDER=help_format
IDIR=include

SLASHSCRIPT='s|\\|\\\\|g;s|\"|\\\"|g'
NEWLINESCRIPT=':a;N;$!ba;s/\n/\\n/g;'

filetocstr()
{
  sed -e $SLASHSCRIPT "$1" | sed $NEWLINESCRIPT
}

gen_line()
{
  name="$(basename "$1")"
  echo "#define $(echo "$name" | tr '[:lower:]' '[:upper:]') \"$(filetocstr "$1")\""
}

help_header()
{
  echo '#ifndef HELP_H
#define HELP_
'
}

help_footer()
{
  echo '
#endif //HELP_H'
}

help_header > "$IDIR/help.h"

for I in help_format/*
do
  gen_line "$I" >> "$IDIR/help.h"
done

help_footer >> "$IDIR/help.h"
