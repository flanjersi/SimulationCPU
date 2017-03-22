
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "cpu.h"
#include "systeme.h"

int current_process = -1;
int nbr_process_alive = 0;
int nbr_process_sleeping = 0;
int nbr_in_getchar = 0;
int nbr_process = 0;
int first_pc = 0;
char tampon = '\0';
char caractere = 'a';

time_t prochain_appel;

/**********************************************************
** Programmes test
***********************************************************/
void make_inst_test_fork(){
    const int R3 = 3;
	make_inst( 0, INST_NOP,   0,   0,   SYSC_IDLE);
    make_inst( 1, INST_NOP,   0,   0,   0);
    make_inst( 2, INST_NOP,   0,   0,   0);
    make_inst( 3, INST_NOP,   0,   0,   0);
    make_inst( 4, INST_JUMP,  0,   0,   0);
    first_pc = SEGMENT_SIZE;
    make_inst( first_pc + 0, INST_SYSC,  0,   0,   SYSC_FORK);
    make_inst( first_pc + 1, INST_IFGT,  0,   0,   6);
    make_inst( first_pc + 2, INST_SYSC,  R3,  0,   SYSC_PUTI);
    make_inst( first_pc + 3, INST_NOP,   0,   0,   0);
	make_inst( first_pc + 4, INST_SYSC,  R3,  0,   SYSC_PUTI);
    make_inst( first_pc + 5, INST_SYSC,  0,   0,   SYSC_EXIT);
    make_inst( first_pc + 6, INST_SUB,   R3,  R3,  -4);
    make_inst( first_pc + 7, INST_SYSC,  R3,  0,   SYSC_SLEEP);
    make_inst( first_pc + 8, INST_SUB,   R3,  R3,  0);
    make_inst( first_pc + 9, INST_JUMP,  0,   0,   0);
}

void make_inst_test_getchar(){
	const int R4 = 4, R3 = 3;
	make_inst( 0, INST_SYSC,  0,   0,   SYSC_IDLE);
	make_inst( 1, INST_NOP,   0,   0,   0);
	make_inst( 2, INST_NOP,   0,   0,   0);
	make_inst( 3, INST_NOP,   0,   0,   0);
	make_inst( 4, INST_JUMP,  0,   0,   0);
	first_pc = SEGMENT_SIZE;
	make_inst( first_pc + 0, INST_SUB,   R3,  R3,  -1);
	make_inst( first_pc + 1, INST_SYSC,  R4,  0,   SYSC_GETCHAR);
	make_inst( first_pc + 2, INST_SYSC,  R4,  0,   SYSC_PUTI);
	make_inst( first_pc + 3, INST_SYSC,  R3,  0,   SYSC_SLEEP);
	make_inst( first_pc + 4, INST_JUMP,  0,   0,   0);
}

void make_inst_test_sleep(){
	const int R3 = 3, R7 = 7;
	make_inst( 0, INST_NOP,   0,   0,   SYSC_IDLE);
	make_inst( 1, INST_NOP,   0,   0,   0);
	make_inst( 2, INST_NOP,   0,   0,   0);
	make_inst( 3, INST_NOP,   0,   0,   0);
	make_inst( 4, INST_JUMP,  0,   0,   0);
	first_pc = SEGMENT_SIZE;
	make_inst(first_pc +  0,   INST_ADD,   R3,  0,   0);
	make_inst(first_pc +  1,   INST_ADD,   R7,  R7,  2);
	make_inst(first_pc +  2,   INST_SYSC,  R7,  0,  SYSC_SLEEP);
	make_inst(first_pc +  3,   INST_SYSC,  R3,  0,  SYSC_PUTI);
	make_inst(first_pc +  4,   INST_NOP,   0,   0,  0);
	make_inst(first_pc +  5,   INST_NOP,   0,   0,  0);
	make_inst(first_pc +  6,   INST_NOP,   0,   0,  0);
	make_inst(first_pc +  7,   INST_NOP,   0,   0,  0);
	make_inst(first_pc +  8,   INST_NOP,   0,   0,  0);
	make_inst(first_pc +  9,   INST_SYSC,  R7,  0,  SYSC_SLEEP);
	make_inst(first_pc +  10,  INST_HALT,  0,   0,  0);
}

void make_inst_test_thread(){
	const int R1 = 1, R3 = 3;
	make_inst(first_pc +  0, INST_SYSC,  R1, R1,   SYSC_NEW_THREAD);
	make_inst(first_pc +  1, INST_IFGT,  0,   0,   10);
	make_inst(first_pc +  2, INST_LOAD,  R3,  0,   0);
	make_inst(first_pc +  3, INST_SYSC,  R3,  0,   SYSC_PUTI);
	make_inst(first_pc +  4, INST_NOP,   0,   0,   0);
	make_inst(first_pc +  5, INST_NOP,   0,   0,   0);
	make_inst(first_pc +  6, INST_NOP,   0,   0,   0);
	make_inst(first_pc +  7, INST_NOP,   0,   0,   0);
	make_inst(first_pc +  8, INST_NOP,   0,   0,   0);
	make_inst(first_pc +  9, INST_HALT,  0,   0,   0);
	make_inst(first_pc + 10, INST_SUB,   R3,  R3,  -3000);
	make_inst(first_pc + 11, INST_STORE, R3,  0,   0);
	make_inst(first_pc + 12, INST_SYSC,  0,   0,   SYSC_EXIT);
}

void make_inst_multi_thread(){
	const int R1 = 1, R3 = 3;
	make_inst( 0, INST_SYSC,  R1,  R1,  SYSC_NEW_THREAD);
	make_inst( 1, INST_IFGT,  0,   0,   10);
	make_inst( 2, INST_SUB,   R3,  R3,  -1000);
	make_inst( 3, INST_SYSC,  R3,  0,   SYSC_PUTI);
	make_inst( 4, INST_NOP,   0,   0,   0);
	make_inst( 5, INST_NOP,   0,   0,   0);
	make_inst( 6, INST_NOP,   0,   0,   0);
	make_inst( 7, INST_NOP,   0,   0,   0);
	make_inst( 8, INST_NOP,   0,   0,   0);
	make_inst( 9, INST_NOP,   0,   0,   0);
	make_inst(10, INST_SUB,   R3,  R3,  -2000);
	make_inst(11, INST_SYSC,  R3,  0,   SYSC_PUTI);
	make_inst(12, INST_SYSC,  0,   0,   SYSC_EXIT);
}


/**********************************************************
** Demarrage du systeme
***********************************************************/
static PSW systeme_init(void) {
	printf("Booting.\n");

	/** Initialisation prochain appel frappe_clavier **/
	prochain_appel = time(NULL) + 4;

	/*** creation d'un programme ***/
	//make_inst_multi_thread();
    make_inst_test_thread();
    //make_inst_test_sleep();
	//make_inst_test_fork();

	/*** valeur initiale du PSW ***/
	for(int i = 0 ; i < MAX_PROCESS ; i++){
		process[i].state = EMPTY;
	}

	/*** Initialisation processus idle ***/
	process[0].state = READY;
	process[0].cpu.PC = 0;
	process[0].cpu.SB = 0;
	process[0].cpu.SS = SEGMENT_SIZE;
	nbr_process_alive++;
	nbr_process++;

	/*** Initialisation de premier processus ***/
	process[1].cpu.PC = 0;
	process[1].cpu.SB = nbr_process * SEGMENT_SIZE;
	process[1].cpu.SS = SEGMENT_SIZE;
	process[1].state = READY;
	nbr_process_alive++;
	nbr_process++;
	current_process = 1;

	return process[1].cpu;
}

PSW systeme_init_boucle(void) {
    PSW cpu;
    const int R1 = 1, R2 = 2, R3 = 3;

    printf("Booting (avec boucle).\n");

    /*** creation d'un programme ***/
    make_inst( 0, INST_ADD,  R1, R1, 0);
    make_inst( 1, INST_ADD,  R2, R2, 10);
    make_inst( 2, INST_ADD,  R3, R3, 5);
    make_inst( 3, INST_CMP,  R1, R2, 0);
    make_inst( 4, INST_IFGT,  0,  0, 11);
    make_inst( 5, INST_NOP,   0,  0, 0);
    make_inst( 6, INST_LOAD,   R2,  R3, 1);
    make_inst( 7, INST_NOP,   0,  0, 0);
    make_inst( 8, INST_ADD,  R1, R3, 0);
    make_inst( 9, INST_SYSC,  R1,  0, SYSC_PUTI);
	make_inst(10, INST_SYSC,  R1,  0, SYSC_NEW_THREAD);
    make_inst(11, INST_JUMP,  0,  0, 3);
    make_inst(12, INST_HALT,  0,  0, 0);

    /*** valeur initiale du PSW ***/
    memset (&cpu, 0, sizeof(cpu));
    cpu.PC = 0;
    cpu.SB = 0;
    cpu.SS = 20;
    return cpu;
}

/**********************************************************
** Affichage registre PC
***********************************************************/
void print_PC(PSW m){
	printf("Program counter : %d\n", m.PC);
}

/**********************************************************
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
void reveil(){
	if(nbr_process_sleeping == 0) return;

	for(int i = 0 ; i < MAX_PROCESS ; i++){
		if(process[i].state == SLEEP){
			if(process[i].wake_up <= time(NULL)){
				process[i].state = READY;
				nbr_process_alive++;
				nbr_process_sleeping--;
			}
		}
	}
}

PSW ordonnanceur(PSW m){
	reveil();

	if(current_process != -1){
		process[current_process].cpu = m;
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

PSW send_thread_to_sleep(PSW m){
	printf("Go to sleep current process %d\n", current_process);

	process[current_process].wake_up = time(NULL) + m.DR[m.RI.i];
	process[current_process].state = SLEEP;
	nbr_process_alive--;
	nbr_process_sleeping++;
	return ordonnanceur(m);
}

void frappe_clavier(){

	if(nbr_in_getchar != 0){
		for(int i = 0 ; i < MAX_PROCESS ; i++){
			if(process[i].state == GETCHAR){
				process[i].state = READY;
				nbr_in_getchar--;
				nbr_process_alive++;
				process[i].cpu.DR[process[i].cpu.RI.i] = caractere;
				return;
			}
		}
	}

	tampon = caractere;
}

PSW my_getchar(PSW m){
	printf("SYSC_GETCHAR\n");

	if(tampon == '\0'){
		process[current_process].state = GETCHAR;
		nbr_in_getchar++;
		return ordonnanceur(m);
	}
	else{
		m.DR[m.RI.i] = tampon;
		tampon = '\0';
		return m;
	}
}


PSW my_fork(PSW m){
	printf("SYSC_FORK\n");
    if(nbr_process >= MAX_PROCESS){
        printf("Nombre max de process atteint, réessaye plus tard.\n");
        return ordonnanceur(m);
    }

    int index = find_first_empty();
    m.AC = index;

    process[index].state = process[current_process].state;
    memcpy(&process[index].cpu, &m, sizeof(m));
    process[index].cpu.SB = index * SEGMENT_SIZE;
    process[index].cpu.AC = 0;

    nbr_process++;

    for(int i = 0 ; i < m.SS ; i++){
        mem[i + process[index].cpu.SB] = mem[i + m.SB];
    }

	return ordonnanceur(m);
}

PSW my_exit(PSW m){
	printf("SYSC_EXIT : End of process\n");
	process[current_process].state = EMPTY;
	nbr_process_alive--;
    nbr_process--;

	return ordonnanceur(m);
}

PSW system_SYSC(PSW m){
	switch(m.RI.ARG){
		case SYSC_EXIT:
			return my_exit(m);
		case SYSC_PUTI:
			printf("SYSC_PUTI : R%d = %d\n", m.RI.i, m.DR[m.RI.i]);
			break;
		case SYSC_NEW_THREAD:
			return new_thread(m);
		case SYSC_SLEEP:
			return send_thread_to_sleep(m);
		case SYSC_IDLE:
			printf("SYSC_IDLE\n");
			break;
		case SYSC_GETCHAR:
			return my_getchar(m);
		case SYSC_FORK:
			return my_fork(m);
		default:
			printf("Unknown ARG of SYSC\n");
			break;
	}
	return m;
}


/**********************************************************
** Simulation du systeme (mode systeme)
***********************************************************/
PSW systeme(PSW m) {
	if(prochain_appel <= time(NULL) && m.IN != INT_INIT){
		prochain_appel = time(NULL) + 4;
		frappe_clavier();
	}

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
			printf("Instuction inconue");
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
