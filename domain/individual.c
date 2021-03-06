//operações no indivíduo


#include "../genalg/genalg.h"


//gera um indivíduo válido
Individual* newindividual()
{
	int i,r,count=0,currenttaskid;
	Edge e;
	list availabletasks = newlist(grafo.n);
	int* predecessorsleft = malloc(grafo.n*sizeof(int));
	Individual* ind = allocateindividual();

	//inicializa e procura nós sem predecessores
	for(i=0;i<grafo.n;i++)
	{
		predecessorsleft[i] = grafo.nodes[i].predqty;
		if(predecessorsleft[i]==0)
		{
			add(availabletasks,i);
		}
	}

	while(availabletasks->size > 0)
	{
		r = rand() % availabletasks->size;
		currenttaskid = at(availabletasks,r);
		erase(availabletasks,r);

		ind->sequence[count] = currenttaskid;

		//if(count==0)
		//	ind->processors[count] = 0;	//obrigar primeiro task a ser alocado no primeiro processador
		//else
		ind->processors[count] = rand()%PROCESSORQTY;
		count++;

		//adiciona à lista nós cujos predecessores já foram escolhidos
		for(e = grafo.nodes[currenttaskid].successors; e!=NULL; e = e->next)
		{
			predecessorsleft[e->node->id]--;
			if(predecessorsleft[e->node->id]==0)
			{
				add(availabletasks,e->node->id);
			}
		}
	}

	free(availabletasks->info);
	free(availabletasks);
	free(predecessorsleft);
	return ind;
}


// static inline int max(int a,int b)
// {
// 	if(a>b)
// 		return a;
// 	return b;
// }


static int makespan(int* arr)
{
	int i,m = arr[0];
	for (i=1;i<PROCESSORQTY;i++)
	{
		if (arr[i] > m)
			m = arr[i];
	}
	return m;
}


void gettasktime(Individual *ind, int taskindex, int* totaltime, int* timestamp)
{
	int processor = ind->processors[taskindex];
	int max = totaltime[processor];
	// int task = ind->sequence[taskindex];
	int t;
	Node* task = &grafo.nodes[taskindex];
	Node* parent;
	Edge e;

	for(e = task->predecessors; e!=NULL; e=e->next)
	{
		parent = e->node;
		if(processor != ind->processors[parent->id]) //if scheduled to different processor
		{
			t = timestamp[parent->id] + parent->cost + e->cost;
			if(t > max)
				max = t;
		}
	}
	timestamp[taskindex] = max;
	totaltime[processor] = max + task->cost;
}


//calcula e seta aptidão do indivíduo
int evaluate(Individual *ind)
{
	int i,task;
	int* totaltime;		//processor time
	int* timestamps;	//task init time

	totaltime = (int*) malloc(PROCESSORQTY*sizeof(int));
	for (i=0;i<PROCESSORQTY;i++)
		totaltime[i] = 0;

	timestamps = (int*)  malloc(grafo.n*sizeof(int));

	for(i=0; i<grafo.n; i++)
	{
		task = ind->sequence[i];
		gettasktime(ind, task, totaltime, timestamps);

		//printf("task: %2d timestamp: %d\n",ind->sequence[i],timestamp[ind->sequence[i]]);
	}
	
	// ind->fitness = max(totaltime[0],totaltime[1]);
	ind->fitness = makespan(totaltime);

	// for(i=0;i<grafo.n;i++)
	// {
	// 	printf("%d %d\n",ind->sequence[i],timestamp[ind->sequence[i]]);
	// }
	
	free(totaltime);
	free(timestamps);
	return ind->fitness;
}


//troca dois genes de lugar
void mutation(Individual *ind)
{
	int i;
	int* genes = (int*)  malloc(grafo.n*sizeof(int));
	char temp[2];
	int a,b;
	a = rand()%grafo.n;
	b = a;
	for(b=a;b==a;b = rand()%grafo.n);
	//printf("%d\t%d\n",a,b);

	for(i=0;i<grafo.n;i++)
		genes[i] = ind->sequence[i];

	temp[0] = ind->sequence[a];
	temp[1] = ind->processors[a];
	ind->sequence[a] = ind->sequence[b];
	ind->processors[a] = ind->processors[b];
	ind->sequence[b] = temp[0];
	ind->processors[b] = temp[1];

	makevalid(ind);

	for(i=0;i<grafo.n;i++)
	{
		if(genes[i] != ind->sequence[i])
		{
			free(genes);
			return;
		}
	}
	free(genes);
	mutation(ind);
}


//retorna 1 se existe a->b
int is_predecessor(int a, int b)
{
	Edge e = grafo.nodes[b].predecessors;
	while(e!=NULL)
	{
		if(e->node->id == a)
			return 1;
		e = e->next;
	}
	return 0;
}


//troca duas tarefas adjacentes de lugar
void mutation_swap(Individual *ind)
{
	int i,a,taskindex,flag=0,aux;
	list positions = newlist(grafo.n);
	for(i=1;i<grafo.n;i++)
		add(positions,i);
	for(;flag==0 && positions->size > 0;)
	{
		a = rand()%positions->size;
		taskindex = at(positions,a);
		erase(positions,a);

		if(!is_predecessor(ind->sequence[taskindex-1], ind->sequence[taskindex]))
		{
			aux = ind->sequence[taskindex-1];
			ind->sequence[taskindex-1] = ind->sequence[taskindex];
			ind->sequence[taskindex] = aux;
			flag = 1;
		}
	}
	free(positions);
}


//troca o processador de uma tarefa
void mutation_proc(Individual *ind)
{
	int a = rand()%grafo.n;
	int b = ind->processors[a];

	while (b == ind->processors[a])
		b = rand()%PROCESSORQTY;
	ind->processors[a] = b;
	// ind->processors[a] = !ind->processors[a];
}


//torna indivíduo válido
void makevalid(Individual *ind)
{
	int i,j;
	list genes = newlist(grafo.n);
	int* newgenes = (int*)  malloc(grafo.n*sizeof(int));
	int* predecessorsleft = (int*)  malloc(grafo.n*sizeof(int));
	Edge e;
	int task;

	for(i=0; i<grafo.n; i++)
	{
		add(genes,ind->sequence[i]);
		predecessorsleft[i] = grafo.nodes[i].predqty;
	}

	for(i=0; i<grafo.n; i++)
	{
		for(j=0;;j++)
		{
			task = at(genes,j);
			if(predecessorsleft[task] == 0)
			{
				newgenes[i] = task;
				erase(genes,j);
				for(e = grafo.nodes[task].successors; e!=NULL; e = e->next)
					predecessorsleft[e->node->id]--;
				break;
			}
		}
	}
	free(genes->info);
	free(genes);
	free(ind->sequence);
	ind->sequence = newgenes;
}


Individual* allocateindividual()
{
	Individual* ind;
	ind = (Individual*) malloc(sizeof(Individual));
	ind->sequence = (int*)  malloc(grafo.n*sizeof(int));
	ind->processors = (int*)  malloc(grafo.n*sizeof(int));
	
	return ind;
}