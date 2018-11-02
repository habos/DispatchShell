#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include "sll.h"
#include "queue.h"

#define TQ 1

enum state{ready, waiting, running};

typedef struct process{
	int arrivalTime;
	int priority;
	int processorTime;
	pid_t pid;
	enum state state;
}PROCESS;

PROCESS *newProcess(int arrivalTime, int priority, int processorTime){
	PROCESS *process = malloc(sizeof(PROCESS));
	process->arrivalTime = arrivalTime;
	process->priority = priority;
	process->processorTime = processorTime;
	process->pid = 0;
	process->state = ready;
	return process;
}

void displayProcess(void *value, FILE *fp) {
	PROCESS *proc = (PROCESS *)value;
	fprintf(fp, "{%d %d %d}", proc->arrivalTime, proc->priority, proc->processorTime);
}

SLL *fillDispatcherList(char* filename, SLL *dispatcherList){
	FILE *fp = fopen(filename, "r");
	const char s[2] = ", ";
	char* token;
	int i, time, prio, exectime, count = 0;
	if(fp != NULL){
		char line[1024];
		while(fgets(line, sizeof line, fp) != NULL){
			token = strtok(line, s);
			for(i = 0; i < 3; i++){
				if(i==0){
					time = atoi(token);
					token = strtok(NULL, s);
				}
				else if(i==1){
					prio = atoi(token);
					token = strtok(NULL, s);
				}
				else{
					exectime = atoi(token);
				}
			}
			PROCESS *process = newProcess(time, prio, exectime);
			insertSLL(dispatcherList, count, process);
			count++;
		}
		fclose(fp);
	} else 
		fprintf(stderr, "Empty file\n");
	return dispatcherList;
}

void terminateProcess(PROCESS *p){
	kill(p->pid, SIGINT);
	waitpid(p->pid, NULL, WUNTRACED);
}

void suspendProcess(PROCESS *p){
	kill(p->pid, SIGTSTP);
	waitpid(p->pid, NULL, WUNTRACED);
	if(p->priority < 3){
		p->priority++;
	}
	p->state = waiting;
}

void startProcess(PROCESS *p){
	pid_t child = fork();
	if(child == 0){
		char *argv[2];
		argv[0] = "./process";
		argv[1] = "20";
		execvp(argv[0], argv);
	}
	else{
		p->pid = child;
	}
	p->state = running;
}

void restartProcess(PROCESS *p){
	kill(p->pid, SIGCONT);
	p->state = running;
}

SLL *getArrivedProcs(SLL *sll, int time){
	SLL *currProcs = newSLL(displayProcess);
	for(int i = 0; i < sizeSLL(sll); i++){
		PROCESS *proc = (PROCESS *)getSLL(sll, i);
		if(proc->arrivalTime == time){
			insertSLL(currProcs, 0, proc);
		}
	}
	return currProcs;
}


int main(int argc, char* argv[]){
	SLL *dispatcherList = newSLL(displayProcess);
	QUEUE *queueArray[4];
	queueArray[0] = newQUEUE();
	queueArray[1] = newQUEUE();
	queueArray[2] = newQUEUE();
	queueArray[3] = newQUEUE();
	int time = 0;
	if(argc != 2){
		fprintf(stderr, "Not the right number of arguements\n");
		return 0;
	}
	dispatcherList = fillDispatcherList(argv[1], dispatcherList);
	PROCESS *currentProcess = 0;
	bool runningSys = false; 
	while(currentProcess || sizeSLL(dispatcherList) > 0){
		//Retrieve all of the processes that are ready to be run
		SLL *currentProcs = getArrivedProcs(dispatcherList, time);
		//fprintf(stdout, "why is this not printing\n");
		//displaySLL(currentProcs, stdout);
		//Place each procs that is ready in its queue by priority
		for(int i = 0; i < sizeSLL(currentProcs); i++){
			PROCESS *temp = (PROCESS *)getSLL(currentProcs, i);
			//displayProcess(temp, stdout);
			enqueue(queueArray[temp->priority], temp);
		}
		//Check if there is a CurrentProcess running that needs to be terminated
		if(currentProcess && currentProcess->processorTime == 0){
			terminateProcess(currentProcess);
			if(runningSys){
				runningSys = false;
			}
			currentProcess = 0;
		}
		//Check if we can run a system process
		if(!runningSys && sizeQUEUE(queueArray[0]) > 0){
			if(currentProcess){
				suspendProcess(currentProcess);
				enqueue(queueArray[currentProcess->priority], currentProcess);
			}
			PROCESS *sysproc = (PROCESS *)dequeue(queueArray[0]);
			startProcess(sysproc);
			currentProcess = sysproc;
			runningSys = true;		
		}
		//Check if a sys process is still running
		else if(runningSys){
			//Do nothing
		}
		//if sys process is not running and there is a process to run in the first priority queue
		else if(sizeQUEUE(queueArray[1]) > 0){
			if(currentProcess){
				suspendProcess(currentProcess);
				enqueue(queueArray[currentProcess->priority], currentProcess);
			}
			PROCESS *proc1 = (PROCESS *)dequeue(queueArray[1]);
			if(proc1->state == ready){
				startProcess(proc1);
			} else {
				restartProcess(proc1);
			}
			currentProcess = proc1;
			runningSys = false;
		}
		else if(sizeQUEUE(queueArray[2]) > 0){
			if(currentProcess){
				suspendProcess(currentProcess);
				enqueue(queueArray[currentProcess->priority], currentProcess);
			}
			PROCESS *proc2 = (PROCESS *)dequeue(queueArray[2]);
			if(proc2->state == ready){
				startProcess(proc2);
			} else {
				restartProcess(proc2);
			}
			currentProcess = proc2;
			runningSys = false;
		}


		else if(sizeQUEUE(queueArray[3]) > 0){
			if(currentProcess){
				suspendProcess(currentProcess);
				enqueue(queueArray[currentProcess->priority], currentProcess);
			}
			PROCESS *proc3 = (PROCESS *)dequeue(queueArray[3]);
			if(proc3->state == ready){
				startProcess(proc3);
			} else {
				restartProcess(proc3);
			}
			currentProcess = proc3;
			runningSys = false;
		}

		if(currentProcess){
			currentProcess->processorTime--;
		}
		//free(currentProcs);
		time++;
		sleep(TQ);
	}
	return 0;
}
