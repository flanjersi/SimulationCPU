
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cpu.h"
#include "systeme.h"


/**********************************************************
** Demarrage du systeme
***********************************************************/

static PSW systeme_init(void) {
	PSW cpu;

	printf("Booting.\n");
	/*** creation d'un programme ***/
	make_inst(0, INST_SUB, 2, 2, -1000); /* R2 -= R2-1000 */
	make_inst(1, INST_ADD, 1, 2, 500);   /* R1 += R2+500 */
	make_inst(2, INST_ADD, 0, 2, 200);   /* R0 += R2+200 */
	make_inst(3, INST_ADD, 0, 1, 100);   /* R0 += R1+100 */
	//make_inst(4, 3, 0, 1, 100);
	/*** valeur initiale du PSW ***/
	memset (&cpu, 0, sizeof(cpu));
	cpu.PC = 0;
	cpu.SB = 0;
	cpu.SS = 20;

	return cpu;
}

PSW systeme_init_boucle(void) {
    PSW cpu;
    const int R1 = 1, R2 = 2, R3 = 3;

    printf("Booting (avec boucle).\n");

    /*** creation d'un programme ***/
    make_inst( 0, INST_ADD,  R1, R1, 0);    /* R1 = 0              */
    make_inst( 1, INST_ADD,  R2, R2, 10);   /* R2 = 1000           */
    make_inst( 2, INST_ADD,  R3, R3, 5);    /* R3 = 5              */
    make_inst( 3, INST_CMP,  R1, R2, 0);    /* AC = (R1 - R2)      */
    make_inst( 4, INST_IFGT,  0,  0, 10);   /* if (AC > 0) PC = 10 */
    make_inst( 5, INST_NOP,   0,  0, 0);    /* no operation        */
    make_inst( 6, INST_NOP,   0,  0, 0);    /* no operation        */
    make_inst( 7, INST_NOP,   0,  0, 0);    /* no operation        */
    make_inst( 8, INST_ADD,  R1, R3, 0);    /* R1 += R3            */
    make_inst( 9, INST_JUMP,  0,  0, 3);    /* PC = 3              */
    make_inst(10, INST_HALT,  0,  0, 0);    /* HALT                */

    /*** valeur initiale du PSW ***/
    memset (&cpu, 0, sizeof(cpu));
    cpu.PC = 0;
    cpu.SB = 0;
    cpu.SS = 20;

    return cpu;
}

/********************************************************
** Affichage registre PC
***********************************************************/

void print_PC(PSW m){
	printf("Program counter : %d\n", m.PC);
}

/********************************************************
** Affichage registre DR
***********************************************************/

void print_DR(PSW m){
	printf("Print data registers : \n");
	for(int i = 0 ; i < 8 ; i++)
		printf("  |--> DR[%d] = %d\n", i , m.DR[i]);
}

void system_SYSC(PSW m){
	switch(m.IR.ARG){
		case SYSC_EXIT:
			exit(0);
			break;
		case SYSC_PUTI:
			printf("SYSC_PUTI : Registre %d -> %d\n", m.IR.i, m.DR[m.IR.i]);
			break;
	}
}

/**********************************************************
** Simulation du systeme (mode systeme)
***********************************************************/

PSW systeme(PSW m) {
	//printf("Received interrupt number : %d\n", m.IN);
	switch (m.IN) {
		case INT_INIT:
			return (systeme_init_boucle());
		case INT_SEGV:
			exit(1);
			break;
		case INT_TRACE:
			printf("\n------ Information Trace Printer ------\n");
			print_PC(m);
			print_DR(m);
			break;
		case INT_INST:
			exit(1);
		case INT_HALT:
			printf("\n---Fin de programme --- \n");
			exit(0);
		case INT_CLOCK:
			printf("\n------ CLOCK ------\n");
			print_PC(m);
			print_DR(m);
			break;
		case INT_SYSC:
			system_SYSC(m);
			break;
	}
	return m;
}
