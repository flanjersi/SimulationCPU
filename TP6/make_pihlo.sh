if test $# -ne 2; then
  echo "Usage : ./make_philo philo {nb_philo}";
  exit;
fi

name=$(basename "$1" .c)
gcc -W -Wall -std=c99 $name.c -o $name -lpthread
./$name $2
rm $name
