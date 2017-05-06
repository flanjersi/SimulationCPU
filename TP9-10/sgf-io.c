
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

void sgf_read_bloc(OFILE* file, int nubloc){
    assert(nubloc < (file->length + BLOCK_SIZE - 1) / BLOCK_SIZE);

    int adr;
    int tmp_nubloc = nubloc;
    int last_block = file->ptr / BLOCK_SIZE;

    if (file->ptr % BLOCK_SIZE == 0) last_block--;
    if (last_block < 0) last_block = 0;

    if (last_block != nubloc){
        if (last_block <= nubloc) {
            tmp_nubloc -= last_block;
            adr = file->current;
        }
        else {
            adr = file->first;
        }
        while (tmp_nubloc-- > 0) {
            assert(adr > 0);
            adr = get_fat(adr);
        }
        file->current = adr;
    }
    read_block(file->current, &file->buffer);
}


/**********************************************************************
PROF. Lire un caract�re dans un fichier ouvert. Cette fonction renvoie
-1 si elle trouve la fin du fichier.
*********************************************************************/

int sgf_getc(OFILE* file){
    assert (file->mode == READ_MODE);
    int c;

    /* d�tecter la fin de fichier */
    if (file->ptr >= file->length) return (-1);

    /* si le buffer est vide, le remplir */
    if (file->ptr % BLOCK_SIZE == 0)
        sgf_read_bloc(file, file->ptr / BLOCK_SIZE);

    c = file->buffer[ (file->ptr % BLOCK_SIZE) ];
    file->ptr ++;

    // printf("OBTENU : ptr : %d length : %d\n", file->ptr, file->length);
    return (c);
}



/**********************************************************************
*
*  FONCTIONS D'ECRITURE DANS UN FICHIER
*
*********************************************************************/

/**********************************************************************
PROF. Ajouter le bloc contenu dans le tampon au fichier ouvert d�crit
par "f".
*********************************************************************/

int sgf_append_block(OFILE* file){
    assert (file->mode == WRITE_MODE || file->mode == APPEND_MODE);

    TBLOCK b;
    int adr = alloc_block();
    if (adr < 0) return (-1);

    if (file->current != FAT_EOF)
        set_fat(file->current, adr);

    set_fat(adr, FAT_EOF);
    file->current = adr;
    file->last    = adr;

    read_block(file->inode, &b.data);
    if (b.inode.first == FAT_EOF)
        b.inode.first = adr;

    b.inode.last   = adr;
    b.inode.length = file->length;

    write_block(file->inode, &b.data);

    return (0);
}


/**********************************************************************
Ecrire le caract�re "c" dans le fichier ouvert d�crit par "file".
*********************************************************************/

void sgf_putc(OFILE* file, char  c){
    assert (file->mode == WRITE_MODE || file->mode == APPEND_MODE);

    // if(file->ptr%BLOCK_SIZE == 0 && file->ptr > 0){
    //     sgf_append_block(file);
    // }

    file->buffer[file->ptr % BLOCK_SIZE] = c;
    file->length += sizeof(char);

    if(file->ptr % BLOCK_SIZE == BLOCK_SIZE - 1) {
        write_block(file->current, &file->buffer);
        TBLOCK b;
        read_block(file->inode, &b.data);
        b.inode.length = file->length;
        write_block(file->inode, &b.data);
    }
    else if(file->ptr % BLOCK_SIZE == 0) {
        sgf_append_block(file);
    }

    file->ptr++;
    // printf("Caractère ecrit : %c ptr : %d \n", c, file->ptr);
}


/**********************************************************************
PROF. �crire la cha�ne de caract�re "s" dans un fichier ouvert en �criture
d�crit par "file".
*********************************************************************/

void sgf_puts(OFILE* file, char* s){
    assert (file->mode == WRITE_MODE || file->mode == APPEND_MODE);

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

void sgf_remove(int  adr_inode){
    TBLOCK b;
    int adr, k;

	read_block(adr_inode, &b.data);

    adr = b.inode.first;
    k   = b.inode.first;

	while(adr != FAT_EOF ){
		k   = adr;
		adr = get_fat(adr);
		set_fat(k, FAT_FREE);
	}

	set_fat(adr_inode, FAT_FREE);

    get_free_fat();
}



/************************************************************
PROF. Ouvrir un fichier en �criture seulement (NULL si �chec).
************************************************************/

static  OFILE*  sgf_open_write(const char* nom){
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
    file->current = FAT_EOF;
    file->inode   = inode;
    file->mode    = WRITE_MODE;
    file->ptr     = 0;

    return (file);
}


static  OFILE*  sgf_open_append(const char* nom){
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
    file->current = b.inode.last;
    file->inode   = inode;
    file->mode    = APPEND_MODE;
    file->ptr     = b.inode.length;

    // if (file->length == 0) {
    //     sgf_close(file);
    //     return sgf_open_write(nom);
    // }

    /* lire le dernier bloc du fichier si la taille n'est pas un multiple de la taille des blocs. */
    if (file->length % BLOCK_SIZE != 0) {
        sgf_read_bloc(file, file->ptr / BLOCK_SIZE);
    }

    return (file);
}

/************************************************************
PROF. Ouvrir un fichier en lecture seulement (NULL si �chec).
************************************************************/

static  OFILE*  sgf_open_read(const char* nom){
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
    file->current = b.inode.first;
    file->inode   = inode;
    file->mode    = READ_MODE;
    file->ptr     = 0;

    return (file);
}


/************************************************************
PROF. Ouvrir un fichier (NULL si �chec).
************************************************************/

OFILE* sgf_open (const char* nom, int mode){
    switch (mode){
        case READ_MODE   : return sgf_open_read(nom);
        case WRITE_MODE  : return sgf_open_write(nom);
        case APPEND_MODE : return sgf_open_append(nom);
        default          : return (NULL);
    }
}


/************************************************************
Fermer un fichier ouvert.
************************************************************/

void sgf_close(OFILE* file){
    // assert(file->mode == WRITE_MODE || file->mode == APPEND_MODE);

    if(file->ptr % BLOCK_SIZE != 0){
        write_block(file->current, &file->buffer);
        TBLOCK b;
        read_block(file->inode, &b.data);
        b.inode.length = file->length;
        write_block(file->inode, &b.data);

        // if(sgf_append_block(file) < 0){
        //     perror("SGF_CLOSE : Probleme sgf_append_block\n");
        //     return;
        // }
    }
    free(file);
    return;
}


/**********************************************************************
PROF. initialiser le SGF
*********************************************************************/

void init_sgf (void){
    init_sgf_disk();
    init_sgf_fat();
}

/**********************************************************************
Accès direct en lecture
*********************************************************************/

int sgf_seek(OFILE* file, int pos){
    assert(file->mode == READ_MODE);

    if(0 > pos || pos >= file->length) return -1;

    int block = (pos - 1) / BLOCK_SIZE;

    int last_block = file->ptr / BLOCK_SIZE;
    if (file->ptr % BLOCK_SIZE == 0)
       --last_block;

    if (block != last_block)
        sgf_read_bloc(file, block);

    file->ptr = pos;

    return 0;
}
