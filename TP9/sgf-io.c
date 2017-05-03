
/*
**  sgf-io.c
**
**  fonctions de lecture/�criture (de caract�res et de blocs)
**  dans un fichier ouvert.
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sgf-disk.h"
#include "sgf-data.h"
#include "sgf-fat.h"
#include "sgf-dir.h"
#include "sgf-io.h"



/**********************************************************************
*
*  FONCTIONS DE LECTURE DANS UN FICHIER
*
*********************************************************************/

/**********************************************************************
Lire dans le "buffer" le bloc logique "nubloc" dans le fichier
ouvert "file".

ATTENTION: Faites en sorte de ne pas recommencer le cha�nage �
partir du bloc 0 si cela n'est pas utile. Pour �viter ce
parcours vous pouvez ajouter un champ � la structure OFILE
qui vous donne l'adresse physique du bloc courant.
*********************************************************************/

void sgf_read_bloc(OFILE* file, int nubloc)
{
    int tmp_nubloc = nubloc;
    int adr;
    assert(nubloc < (file->length + BLOCK_SIZE - 1) / BLOCK_SIZE);

    adr = file->first;

    while (tmp_nubloc > 0) {
        adr = get_fat(adr);

        assert(adr > 0);
        tmp_nubloc--;
    }

    read_block(adr, &file->buffer);
}


/**********************************************************************
Lire un caract�re dans un fichier ouvert. Cette fonction renvoie
-1 si elle trouve la fin du fichier.
*********************************************************************/

int sgf_getc(OFILE* file)
{
    int c;

    assert (file->mode == READ_MODE);

    /* d�tecter la fin de fichier */
    if (file->ptr >= file->length)
      return (-1);

    /* si le buffer est vide, le remplir */
    if ((file->ptr % BLOCK_SIZE) == 0)
    {
        sgf_read_bloc(file, file->ptr / BLOCK_SIZE);
    }

    c = file->buffer[ (file->ptr % BLOCK_SIZE) ];
    file->ptr ++;
    return (c);
}



/**********************************************************************
*
*  FONCTIONS D'ECRITURE DANS UN FICHIER
*
*********************************************************************/

/**********************************************************************
Ajouter le bloc contenu dans le tampon au fichier ouvert d�crit
par "f".
*********************************************************************/

int sgf_append_block(OFILE* f)
{
    TBLOCK b;
    int adr = alloc_block();

    if (adr < 0) return (-1);

    write_block(adr, & f->buffer );
    set_fat(adr, FAT_EOF);

    if (f->first == FAT_EOF) {
        f->first = adr;
        f->last = adr;
    }
    else {
        set_fat(f->last, adr);
        f->last = adr;
    }

    b.inode.length = f->ptr;
    b.inode.first = f->first;
    b.inode.last = f->last;

    write_block(f->inode, &b.data);

    return (0);
}


/**********************************************************************
Ecrire le caract�re "c" dans le fichier ouvert d�crit par "file".
*********************************************************************/

void sgf_putc(OFILE* file, char  c)
{
    assert (file->mode == WRITE_MODE);

    if(file->ptr%BLOCK_SIZE == 0 && file->ptr > 0){
        sgf_append_block(file);
    }

    file->buffer[file->ptr%BLOCK_SIZE] = c;
    file->ptr++;
    file->length += sizeof(char);

    // printf("Caractère ecrit : %c\n",c);
}


/**********************************************************************
�crire la cha�ne de caract�re "s" dans un fichier ouvert en �criture
d�crit par "file".
*********************************************************************/

void sgf_puts(OFILE* file, char* s)
{
    assert (file->mode == WRITE_MODE);

    for (; (*s != '\0'); s++) {
        sgf_putc(file, *s);
    }
}



/**********************************************************************
*
*  FONCTIONS D'OUVERTURE, DE FERMETURE ET DE DESTRUCTION.
*
*********************************************************************/

/************************************************************
D�truire un fichier.
************************************************************/

void sgf_remove(int  adr_inode)
{
    TBLOCK b;
    int adr, k;

	read_block(adr_inode,&b.data);

    adr = b.inode.first;
    k = b.inode.first;

	while(adr != FAT_EOF ){
		k = adr;
		adr = get_fat(adr);
		set_fat(k,FAT_FREE);
	}

	set_fat(adr_inode,FAT_FREE);

    get_free_fat();
}


/************************************************************
Ouvrir un fichier en �criture seulement (NULL si �chec).
************************************************************/

static  OFILE*  sgf_open_write(const char* nom)
{
    int inode, oldinode;
    OFILE* file;
    TBLOCK b;

    /* Rechercher un bloc libre sur disque */
    inode = alloc_block();
    assert (inode >= 0);

    /* Allouer une structure OFILE */
    file = malloc(sizeof(struct OFILE));
    if (file == NULL) return (NULL);

    /* pr�parer un inode vers un fichier vide */
    b.inode.length = 0;
    b.inode.first  = FAT_EOF;
    b.inode.last   = FAT_EOF;

    /* sauver ce inode */
    write_block(inode, &b.data);
    set_fat(inode, FAT_INODE);

    /* mettre a jour le repertoire */
    oldinode = add_inode(nom, inode);
    if (oldinode > 0) sgf_remove(oldinode);

    file->length  = 0;
    file->first   = FAT_EOF;
    file->last    = FAT_EOF;
    file->inode   = inode;
    file->mode    = WRITE_MODE;
    file->ptr     = 0;

    return (file);
}


/************************************************************
Ouvrir un fichier en lecture seulement (NULL si �chec).
************************************************************/

static  OFILE*  sgf_open_read(const char* nom)
{
    int inode;
    OFILE* file;
    TBLOCK b;

    /* Chercher le fichier dans le r�pertoire */
    inode = find_inode(nom);
    if (inode < 0) return (NULL);

    /* lire le inode */
    read_block(inode, &b.data);

    /* Allouer une structure OFILE */
    file = malloc(sizeof(struct OFILE));
    if (file == NULL) return (NULL);

    file->length  = b.inode.length;
    file->first   = b.inode.first;
    file->last    = b.inode.last;
    file->inode   = inode;
    file->mode    = READ_MODE;
    file->ptr     = 0;

    return (file);
}


/************************************************************
Ouvrir un fichier (NULL si �chec).
************************************************************/

OFILE* sgf_open (const char* nom, int mode)
{
    switch (mode)
    {
        case READ_MODE:  return sgf_open_read(nom);
        case WRITE_MODE: return sgf_open_write(nom);
        default:         return (NULL);
    }
}


/************************************************************
Fermer un fichier ouvert.
************************************************************/

void sgf_close(OFILE* file)
{
    if(file->mode == WRITE_MODE ){
        if((file->ptr%BLOCK_SIZE)!=0){
            if(sgf_append_block(file) < 0){
                perror("SGF_CLOSE : Probleme sgf_append_block\n");
                return;
            }
        }
    }
    free(file);
    return;
}


/**********************************************************************
initialiser le SGF
*********************************************************************/

void init_sgf (void)
{
    init_sgf_disk();
    init_sgf_fat();
}

/**********************************************************************
Accès direct en lecture
*********************************************************************/

int sgf_seek(OFILE* f, int pos){
    if(f == NULL || f->length < pos) return -1;

    f->ptr         = pos;
    int curr_block = f->ptr / BLOCK_SIZE;

    sgf_read_bloc(f, curr_block);

    return 0;
}
