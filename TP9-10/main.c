
/*
**  main.c
**
**  Utilisation des fonctions du mini SGF.
**
**  04/04/2007
*/

#include <stdio.h>
#include <stdlib.h>

#include "sgf-disk.h"
#include "sgf-fat.h"
#include "sgf-dir.h"
#include "sgf-io.h"

void empty_file(OFILE* file){
    file = sgf_open("essai.txt", WRITE_MODE);
    sgf_close(file);
}

void read_file(OFILE* file, int c){
    file = sgf_open("essai.txt", READ_MODE);
    while ((c = sgf_getc(file)) > 0)
        putchar(c);
    sgf_close(file);
}

void append_file(OFILE* file, int c){
    file = sgf_open("essai.txt", APPEND_MODE);
    sgf_putc(file, c);
    sgf_close(file);
}

void mutliple_append_file(OFILE* file, int count){
    for (int i = 0 ; i < count ; i++)
        append_file(file, 'c');
}

void write_file(OFILE* file){
    file = sgf_open("essai.txt", WRITE_MODE);
    sgf_puts(file, "Ceci est un petit texte qui occupe\n");
    sgf_puts(file, "quelques blocs sur ce disque fictif.\n");
    sgf_puts(file, "Le bloc faisant 128 octets, il faut\n");
    sgf_puts(file, "que je remplisse pour utiliser\n");
    sgf_puts(file, "plusieurs blocs.\n");
    sgf_close(file);
}

void seek_8_file(OFILE* file){
    file = sgf_open("essai.txt", READ_MODE);
    int i = 0;
    while(sgf_seek(file, 8 * i) != -1){
        char c = sgf_getc(file);
        putchar(c);
        putchar(' ');
        i++;
    }
    printf("\n");
    sgf_close(file);
}

void final_write_file(OFILE* file){
    file = sgf_open("essai.txt", WRITE_MODE);
    sgf_write(file, "Ceci est un petit texte qui occupe quelques blocs sur ce disque fictif. Le bloc faisant 128 octets, il faut que je remplisse pour utiliser plusieurs blocs.", 155);
    sgf_close(file);
}

int main() {
    OFILE* file;
    int c;

    init_sgf();

    printf("\n>> Listing du disque\n");
    list_directory();

    printf("\n>> Lecture du fichier\n");
    read_file(file, c);

    printf("\n>> Recherche tous les 8 caractères\n");
    seek_8_file(file);

    printf("\n>> Ecriture dans fichier\n");
    write_file(file);
    read_file(file, c);

    printf("\n>> 500 ajouts dans fichier\n");
    empty_file(file);
    mutliple_append_file(file, 500);
    read_file(file, c);

    printf("\n>> Ecriture final\n");
    empty_file(file);
    final_write_file(file);
    read_file(file, c);

    printf("\n");
    return (EXIT_SUCCESS);
}
