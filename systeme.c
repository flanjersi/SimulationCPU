
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cpu.h"
#include "systeme.h"

int current_process = -1;
/**********************************************************
** Demarrage du systeme
***********************************************************/

static PSW systeme_init(void) {
	PSW cpu, cpu2;
	const int R1 = 1, R2 = 2, R3 = 3;

	printf("Booting.\n");

	/*** creation d'un programme2 ***/
	make_inst( 0, INST_ADD,  R1, R1, 0);    /* R1 = 0              */
	make_inst( 1, INST_ADD,  R2, R2, 100);   /* R2 = 1000           */
	make_inst( 2, INST_ADD,  R3, R3, 5);    /* R3 = 5              */
	make_inst( 3, INST_CMP,  R1, R2, 0);    /* AC = (R1 - R2)      */
	make_inst( 4, INST_IFGT,  0,  0, 12);   /* if (AC > 0) PC = 11 */
	make_inst( 5, INST_NOP,   0,  0, 0);    /* no operation        */
	make_inst( 6, INST_LOAD,   R2,  R3, 1);    /* no operation        */
	make_inst( 7, INST_NOP,   0,  0, 0);    /* no operation        */
	make_inst( 8, INST_ADD,  R1, R3, 0);    /* R1 += R3            */
	make_inst( 9, INST_SYSC,  R1,  0, SYSC_PUTI);    /* SYSCALL    */
	make_inst(10, INST_SYSC,  R1,  0, SYSC_NEWTHREAD);    /* SYSCALL    */
    make_inst(11, INST_JUMP,  0,  0, 3);    /* PC = 3              */
    make_inst(12, INST_HALT,  0,  0, 0);    /* HALT                */

	/*** valeur initiale du PSW ***/
	memset (&cpu, 0, sizeof(cpu));
	cpu.PC = 0;
	cpu.SB = 0;
	cpu.SS = 20;

	memset (&cpu2, 0, sizeof(cpu));
	cpu2.PC = 0;
	cpu2.SB = 0;
	cpu2.SS = 20;


	/*** Initialisation de premier processus ***/
	memcpy(&(process[0].cpu), &cpu, sizeof(PSW));
	process[0].state = EMPTY;

	memcpy(&(process[1].cpu), &cpu2, sizeof(PSW));
	process[1].state = READY;

	current_process = 0;
	return process[0].cpu;
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
    make_inst( 4, INST_IFGT,  0,  0, 11);   /* if (AC > 0) PC = 11 */
    make_inst( 5, INST_NOP,   0,  0, 0);    /* no operation        */
    make_inst( 6, INST_LOAD,   R2,  R3, 1);    /* no operation        */
    make_inst( 7, INST_NOP,   0,  0, 0);    /* no operation        */
    make_inst( 8, INST_ADD,  R1, R3, 0);    /* R1 += R3            */
    make_inst( 9, INST_SYSC,  R1,  0, SYSC_PUTI);    /* SYSCALL    */
	make_inst(10, INST_SYSC,  R1,  0, SYSC_NEWTHREAD);    /* SYSCALL    */
    make_inst(11, INST_JUMP,  0,  0, 3);    /* PC = 3              */
    make_inst(12, INST_HALT,  0,  0, 0);    /* HALT                */

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
	switch(m.RI.ARG){
		case SYSC_EXIT:
			printf("SYSC_EXIT : End of program\n");
			exit(0);
			break;
		case SYSC_PUTI:
			printf("ORDONNANCEUR : %d\n", current_process);
			printf("SYSC_PUTI : R%d = %d\n", m.RI.i, m.DR[m.RI.i]);
			break;
		case SYSC_NEWTHREAD:
			printf("New thread");
			break;
		default:
			printf("Unknown ARG of SYSC");
			break;
	}
}

/**********************************************************
** Simulation du systeme (mode systeme)
***********************************************************/

PSW ordonnanceur(PSW m){
	if(current_process != -1){
		memcpy(&(process[current_process].cpu), &m, sizeof(PSW));
		process[current_process].state = READY;
	}

	do{
		current_process = (current_process + 1) % MAX_PROCESS;
	}while(process[current_process].state != READY);

	process[current_process].state = EMPTY;
	return process[current_process].cpu;
}

/**********************************************************
** Simulation du systeme (mode systeme)
***********************************************************/

PSW systeme(PSW m) {
	//printf("Received interrupt number : %d\n", m.IN);
	switch (m.IN) {
		case INT_INIT:
			return (systeme_init());
		case INT_SEGV:
			printf("INT_SEGV error\n");
			exit(1);
			break;
		case INT_TRACE:
			printf("\n------ Information Trace Printer ------\n");
			print_PC(m);
			print_DR(m);
			break;
		case INT_INST:
			exit(EXIT_FAILURE);
		case INT_HALT:
			printf("\n---Fin de programme --- \n");
			exit(EXIT_SUCCESS);
		case INT_CLOCK:
			return ordonnanceur(m);
			break;
		case INT_SYSC:
			printf("\n------ SYSCALL ------\n");
			system_SYSC(m);
			break;
	}
	return m;
}
