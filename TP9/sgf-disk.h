
#ifndef __DRIVER_DISK__
#define __DRIVER_DISK__


/************************************************************
 *
 *  ROUTINES DE BASE POUR LES E/S DE BLOCKS.
 *
 ***********************************************************/

/**********************************************************************
 *
 *  D�finition d'un bloc. Le bloc est l'unit� �l�mentaire
 *  d'entr�e/sortie sur les supports. Le mat�riel est capable de
 *  lire et d'�crire un bloc sur l'unit�.
 *
 *********************************************************************/

#define BLOCK_SIZE              (128)    /* 128 octets */

typedef char BLOCK[ BLOCK_SIZE ];


/************************************************************
 R�cup�rer la taille du disque (en blocs)
 ***********************************************************/

int get_disk_size();

char* get_disk_name();


/************************************************************
 Primitives de bas niveau pour lire ou �crire un bloc sur
 disque. ATTENTION: pour simplifier, les E/S sont synchrones.
 En d'autres termes, ces proc�dures ATTENDENT la fin de l'E/S
 ***********************************************************/

void read_block (int n, BLOCK* b);
void write_block (int n, BLOCK* b);

/************************************************************
 initialisation et d�couverte du disque
 ***********************************************************/

void init_sgf_disk (void);


/************************************************************
 Afficher le message d'erreur et stopper la simulation.
 ************************************************************/

void panic (const char *format, ...);

#endif
