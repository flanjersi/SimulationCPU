
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "cpu.h"
#include "systeme.h"

int current_process = -1;
int nbr_process_alive = 0;
int nbr_process_sleeping = 0;
/**********************************************************
** Demarrage du systeme
***********************************************************/
void make_inst_multi_thread_store(){
	const int R1 = 1, R2 = 2, R3 = 3;

	/*** Exemple de création d'un thread ***/
	make_inst( 0, INST_SYSC,  R1, R1, SYSC_NEW_THREAD);  /* créer un thread  */
	make_inst( 1, INST_IFGT,   0,  0, 10);               /* le père va en 10 */

	/*** code du fils ***/
	make_inst( 2, INST_LOAD,  R3, 0, 0);            /* R3 = 1000    */
	make_inst( 3, INST_SYSC,  R3,  0, SYSC_PUTI);        /* afficher R3  */
	make_inst( 4, INST_NOP, 0,  0, 0);
	make_inst( 5, INST_NOP,   0,   0, 0);
	make_inst( 6, INST_NOP,   0,   0, 0);
	make_inst( 7, INST_NOP,   0,   0, 0);
	make_inst( 8, INST_NOP,   0,   0, 0);
	make_inst( 9, INST_HALT,   0,   0, 0);

	/*** code du père ***/
	make_inst(10, INST_SUB, R3, R3, -3000);           /* R3 = 2000     */
	make_inst(11, INST_STORE, R3,  0, 0);       /* afficher R3   */
	make_inst(12, INST_SYSC,   0,  0, SYSC_EXIT);       /* fin du thread */
}

void make_inst_multi_thread(){
	const int R1 = 1, R2 = 2, R3 = 3;

		/*** Exemple de création d'un thread ***/
	make_inst( 0, INST_SYSC,  R1, R1, SYSC_NEW_THREAD);  /* créer un thread  */
	make_inst( 1, INST_IFGT,   0,  0, 10);               /* le père va en 10 */

	/*** code du fils ***/
	make_inst( 2, INST_SUB,   R3, R3, -1000);            /* R3 = 1000    */
	make_inst( 3, INST_SYSC,  R3,  0, SYSC_PUTI);        /* afficher R3  */
	make_inst( 4, INST_NOP,   0,   0, 0);
	make_inst( 5, INST_NOP,   0,   0, 0);
	make_inst( 6, INST_NOP,   0,   0, 0);
	make_inst( 7, INST_NOP,   0,   0, 0);
	make_inst( 8, INST_NOP,   0,   0, 0);
	make_inst( 9, INST_NOP,   0,   0, 0);

	/*** code du père ***/
	make_inst(10, INST_SUB,   R3, R3, -2000);           /* R3 = 2000     */
	make_inst(11, INST_SYSC,  R3,  0, SYSC_PUTI);       /* afficher R3   */
	make_inst(12, INST_SYSC,   0,  0, SYSC_EXIT);       /* fin du thread */
}



static PSW systeme_init(void) {
	PSW cpu;

	printf("Booting.\n");

	/*** creation d'un programme2 ***/
	make_inst_multi_thread();
	//make_inst_multi_thread_store();
	/*** valeur initiale du PSW ***/
	memset (&cpu, 0, sizeof(cpu));
	cpu.PC = 0;
	cpu.SB = 0;
	cpu.SS = 20;

	for(int i = 0 ; i < MAX_PROCESS ; i++){
		process[i].state = EMPTY;
	}
	/*** Initialisation processus idle ***/
	process[0].state = READY;
	process[0].cpu.PC = 0;
	process[0].cpu.SB = 0;
	process[0].cpu.SS = 20;
	nbr_process_alive++;


	/*** Initialisation de premier processus ***/
	memcpy(&(process[1].cpu), &cpu, sizeof(PSW));
	process[1].state = READY;
	nbr_process_alive++;

	current_process = 1;
	return process[1].cpu;
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
	  make_inst(10, INST_SYSC,  R1,  0, SYSC_NEW_THREAD);    /* SYSCALL    */
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

int find_first_empty(){
	for(int i = 0 ; i < MAX_PROCESS ; i++){
		if(process[i].state == EMPTY)
			return i;
	}
	return -1;
}

/**********************************************************
** Simulation du systeme (mode systeme)
***********************************************************/

PSW ordonnanceur(PSW m){
	if(current_process != -1){
		memcpy(&(process[current_process].cpu), &m, sizeof(PSW));
	}

	do{
		current_process = (current_process + 1) % MAX_PROCESS;
	}while(process[current_process].state != READY);

	return process[current_process].cpu;
}

PSW new_thread(PSW m){
	int index = find_first_empty();
	printf("Thread créé : %d\n", index);
	m.DR[m.RI.i] = index;
	m.AC = index;

	process[index].cpu = m;
	process[index].cpu.DR[m.RI.i] = 0;
	process[index].cpu.AC = 0;
	process[index].state = READY;
	nbr_process_alive++;
	return m;
}


PSW sleeping_thread(PSW m){
	sleeping_process[nbr_process_sleeping].id_process = current_process;
	sleeping_process[nbr_process_sleeping].wake_up = time(NULL) + m.DR[m.RI.i];

	process[current_process].state = SLEEP;

	return ordonnanceur(m);
}


PSW system_SYSC(PSW m){
	switch(m.RI.ARG){
		case SYSC_EXIT:
			printf("SYSC_EXIT : End of process\n");
			process[current_process].state = EMPTY;
			return ordonnanceur(m);
		case SYSC_PUTI:
			printf("SYSC_PUTI : R%d = %d\n", m.RI.i, m.DR[m.RI.i]);
			break;
		case SYSC_NEW_THREAD:
			return new_thread(m);
		case SYSC_SLEEP:
			return sleeping_thread(m);
		default:
			printf("Unknown ARG of SYSC");
			break;
	}

	return m;
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
			return system_SYSC(m);
			break;
	}
	return m;
}
