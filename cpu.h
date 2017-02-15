
#ifndef __CPU_H
#define __CPU_H


/**********************************************************
** Codes associes aux interruptions
***********************************************************/

#define INT_INIT	(1)
#define INT_SEGV	(2)
#define INT_INST	(3)
#define INT_TRACE	(4)


/**********************************************************
** Codes associes aux instructions
***********************************************************/

#define INST_ADD	(0)
#define INST_SUB	(1)
#define INST_CMP	(2)


/**********************************************************
** definition d'un mot memoire
***********************************************************/

typedef int WORD;         /* un mot est un entier 32 bits  */

extern WORD mem[128];     /* memoire                       */


/**********************************************************
** Codage d'une instruction (32 bits)
***********************************************************/

typedef struct {
	unsigned OP: 10;  /* code operation (10 bits)  */
	unsigned i:   3;  /* nu 1er registre (3 bits)  */
	unsigned j:   3;  /* nu 2eme registre (3 bits) */
	short    ARG;     /* argument (16 bits)        */
} INST;


/**********************************************************
** Le Mot d'Etat du Processeur (PSW)
***********************************************************/

typedef struct PSW {    /* Processor Status Word */
	WORD PC;        /* Program Counter */
	WORD SB;        /* Segment Base */
	WORD SS;        /* Segment Size */
	WORD IN;        /* Interrupt number */
	WORD DR[8];     /* Data Registers */
	WORD AC;        /* Accumulateur */
	INST RI;        /* Registre instruction */
} PSW;


/**********************************************************
** implanter une instruction en memoire
***********************************************************/

void make_inst(int adr, unsigned code, unsigned i, unsigned j, short arg);


/**********************************************************
** executer un code en mode utilisateur
***********************************************************/

PSW cpu(PSW);


#endif
