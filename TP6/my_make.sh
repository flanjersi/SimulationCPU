if test $# -ne 1; then
  echo "Usage : ./my_make mon_fichier_sans_extension";
  exit;
fi

name=$(basename "$1" .c)
gcc -W -Wall -std=c99 $name.c -o $name -lpthread
./$name
