#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include "timer.h"


pthread_mutex_t mutex;

typedef struct
{
	int n_threads,v_size,n_tests;
}test;	//parametros para o teste

typedef struct
{
	test* tests;
	int size,limit;
}test_v;	//estrutura que facilita o uso de vetores em c

void init_test(test_v* t)//inicia o vetor de testes
{
	t->size = 0;
	t->limit = 10;
	t->tests = malloc(sizeof(test) * t->limit);
}

void set_test(test* t,int n_threads,int v_size,int n_tests)//facilita a atribuicao de testes
{
	t->n_threads = n_threads;
	t->v_size = v_size;
	t->n_tests = n_tests;
}

void add_test(test_v* _vtest,test t)//adiciona teste a lista de testes
{
	if(_vtest->size >= _vtest->limit)
	{
		test* temp = _vtest->tests;
		_vtest->tests = malloc(sizeof(test) * 2 * _vtest->limit);
		for(int i = 0; i < _vtest->limit; i++) _vtest->tests[i] = temp[i];
		free(temp);
		_vtest->limit *= 2;
	}
	_vtest->tests[_vtest->size++] = t;
}

void add_tfacil(test_v* tv,int n_threads, int v_size,int n_tests)//facilita a adicao de testes a lista de testes
{
	test t;
	t.n_threads = n_threads;
	t.v_size = v_size;
	t.n_tests = n_tests;
	add_test(tv,t);
}




int quick_sort_step(int* v,int inicio,int fim)//metodo que realiza uma passada do quick sort
{
	int pivo = fim - 1,menores = inicio,maiores = inicio;
	if(fim != inicio)
	{
		int temp;
		for(;maiores < pivo;maiores++)
		{
			if(v[maiores] <= v[pivo])
			{
				temp = v[maiores];
				v[maiores] = v[menores];
				v[menores] = temp;
				menores++;				
			}
		}
		temp = v[menores];
		v[menores] = v[pivo];
		v[pivo] = temp;
		return menores;//retorna a posicao final do pivo
	}
	return -1;//retorna -1 se o vetor for unitario
	
} 

typedef struct 
{
	int size,limit;
	int* array;
}vector_i;	//estrutura para facilitar o uso de vetores em c

typedef struct
{
	int size,limit;
	pthread_t* array;
}vector_pt;	//estrutura para facilitar o uso de vetores em c

void vector_i_init(vector_i* v)//inicializa o vetor
{
	v->array = malloc(sizeof(int) * 64 );
	v->limit = 64;
	v->size = 0;
}

void vector_pt_init(vector_pt* v)
{
	v->array = malloc(sizeof(pthread_t) * 64 );
	v->limit = 64;
	v->size = 0;
}


void vector_pt_add(vector_pt* v)//reserva posicao no vetor para uso
{
	if(v->size >= v->limit)
	{
		pthread_t* temp = malloc(sizeof(pthread_t) * v->limit * 2);
		for(int i = 0; i < v->limit; i++)
		{
			temp[i] = v->array[i];
		} 
		free(v->array);
		v->array = temp;
		v->limit *= 2;
	}	
	v->size++;
}


void vector_i_add(vector_i* v,int x)//adciona x ao vetor v
{
	if(v->size >= v->limit)
	{
		int* temp = malloc(sizeof(int) * v->limit * 2);
		for(int i = 0; i < v->limit; i++)
		{
			temp[i] = v->array[i];
		} 
		free(v->array);
		v->array = temp;
		v->limit *= 2;
	}	
	v->array[v->size] = x;
	v->size++;
}

vector_i ids;// vetor com os estados de cada thread criada, 1 para ativa e -1 para encerrada
vector_pt t_ids;//vetor para controle das threads (pthread_t)

void quick_sort_seq(int*  v,int inicio,int fim)//quick sort recursivo sequencial 
{
	int pivo = fim - 1;
	int menores = inicio,maiores = inicio;
	if(inicio < pivo)
	{
		int temp;
		for(;maiores < pivo;maiores++)
		{
			if(v[maiores] <= v[pivo])
			{

				temp = v[maiores];
				v[maiores] = v[menores];
				v[menores] = temp;
				menores++;				
			}
		}
		temp = v[menores];
		v[menores] = v[pivo];
		v[pivo] = temp;
		quick_sort_seq(v,inicio,menores);
		quick_sort_seq(v,menores + 1, fim);
	}
}

void randon_array(int* v,int size) //preenche o vetor com elementros aleatorios entre 0 e o tamanho do vetor
{
	for(int i = 0; i < size; i++)
		v[i] = rand() % size;
}

void print_array(int* v, int size)
{
	for(int i = 0; i < size; i++) printf("v[%d]: %d\n",i,v[i]);
}

typedef struct//estrutura a ser usada como argumento para as threads
{
	int id,inicio,fim,n_threads,v_size;
	int* v;
}t_args;

void* new_qs(void* args);

void cria_thread(int* v,int inicio,int fim,int v_size,int n_threads)//funcao que facilita a criacao de threads
{
	t_args* args = malloc(sizeof(t_args));
	args->inicio = inicio;
	args->fim = fim;
	args->id = ids.size;
	args->v = v;
	args->n_threads = n_threads;
	args->v_size = v_size;
	int ts = t_ids.size;
	vector_pt_add(&t_ids);
	vector_i_add(&ids,0);
	pthread_create(&t_ids.array[ts],NULL,(void*) new_qs,(void*) args);	
}

void* new_qs(void* _args)//metodo concorrente
{
	t_args args = *(t_args*) _args;
	int inicio = args.inicio,fim = args.fim,id = args.id,v_size = args.v_size,n_threads = args.n_threads;
	int* v = args.v;
	//printf("Thread %d iniciada\n",id);
	ids.array[id] = 1;
	while(inicio < fim)
	{
		if(fim - inicio <=(float) v_size/n_threads)
		{
			quick_sort_seq(v,inicio,fim);//quando o vetor e dividido o suficiente cada thread ordena o resto de forma sequencial
			inicio = fim;
		}
		else
		{
			int pivo = quick_sort_step(v,inicio,fim);
			if(pivo == -1) break;
			//if(pivo == -1) inicio = fim;
			pthread_mutex_lock(&mutex);
			if(inicio < pivo)cria_thread(v,inicio,pivo,v_size,n_threads);
			pthread_mutex_unlock(&mutex);
			inicio = pivo + 1;

		}
	}	
	//if(ids.array[id] == -1) printf("ja estava encerrada: %d\n",id);
	ids.array[id] = -1;//avisa o fim da thread
	free(_args);
	//printf("Thread %d encerrada\n",id);
	pthread_exit(NULL);
}

int verifica_array(int* v, int size)
{
	for(int i = 1; (i < size); i++)
	{
		if(v[i] < v[i-1]) return 0;
	}
	return 1;
}


void teste(test t)
{
	int* v = malloc(sizeof(int) * t.v_size);
	double acc = 0;
	printf("Inicio do teste (threads %d, tamanho do vetor %d, numero de testes %d) \n",t.n_threads,t.v_size,t.n_tests);
	for(int i = 0; i < t.n_tests; i++)
	{
		vector_i_init(&ids);
		vector_pt_init(&t_ids);
		//printf("subteste %d\n",i);
		randon_array(v,t.v_size);
		//printf("Vetor inicial:\n");
		//verifica_array(v,t.v_size);
		double t0,t1,seq;
		//printf("Sequencial\n");
		GET_TIME(t0);
		quick_sort_seq(v,0,t.v_size);
		GET_TIME(t1);
		seq = t1 -t0;
		//printf("Vetor final seq\n");
		//verifica_array(v,t.v_size);
		randon_array(v,t.v_size);
		//printf("Concorrente\n");
		GET_TIME(t0);
		cria_thread(v,0,t.v_size,t.v_size,t.n_threads);
		int fim = 0;
		//printf("loop\n");
		while (!fim )//verifica se todas as threads terminaram
		{
			fim = 1;
			int x = ids.size;
			for(int i = 0; i < x; i++)
			{
				int t = ids.array[i] == -1;
				fim = fim && t;
				//if(!t) printf("Thread %d ainda nao terminou, estado: %d \n",i,ids.array[i]);
			
			}
		}
		GET_TIME(t1);
		acc += seq/(t1-t0);
		//printf("Vetor final:\n");
		//print_array(v,V_SIZE);
		//verifica_array(v,t.v_size);
		//printf("Tamanho do array: %d\n",t.v_size);
		//printf("Tempo sequencial: %f\n",seq);
		//printf("Tempo concorrente: %f\n",t1 -t0);
		//printf("Acc: %f\n",seq/(t1-t0));
		free(ids.array);
		free(t_ids.array);
	}	
	//printf("Fim do teste (threads %d, tamanho do vetor %d, numero de testes %d) \n",t.n_threads,t.v_size,t.n_tests);
	printf("Aceleracao media: %f\n",acc/t.n_tests);
	printf("----------------------------------\n");
	free(v);
	
}

int main()
{
	pthread_mutex_init(&mutex,NULL);
	
	srand(time(NULL));
	test_v tv;
	init_test(&tv);
	int MIL = 1000,MILHAO = 1000000;
	//adiciona testes a fila
	add_tfacil(&tv,2,MIL,50);//realiza 50 testes com 2 threads 1000 posicoes no vetor
	add_tfacil(&tv,4,MIL,50);
	add_tfacil(&tv,8,MIL,50);
	add_tfacil(&tv,16,MIL,50);
	
	add_tfacil(&tv,2,10*MIL,30);
	add_tfacil(&tv,4,10*MIL,30);
	add_tfacil(&tv,8,10*MIL,30);
	add_tfacil(&tv,16,10*MIL,30);
	
	add_tfacil(&tv,2,50*MIL,30);
	add_tfacil(&tv,4,50*MIL,30);
	add_tfacil(&tv,8,50*MIL,30);
	add_tfacil(&tv,16,50*MIL,30);
	
	add_tfacil(&tv,2,100*MIL,20);
	add_tfacil(&tv,4,100*MIL,20);
	add_tfacil(&tv,8,100*MIL,20);
	add_tfacil(&tv,16,100*MIL,20);
	add_tfacil(&tv,32,100*MIL,20);
	
	add_tfacil(&tv,2,MILHAO,10);
	add_tfacil(&tv,4,MILHAO,10);
	add_tfacil(&tv,8,MILHAO,10);
	add_tfacil(&tv,16,MILHAO,10);
	add_tfacil(&tv,32,MILHAO,10);
	add_tfacil(&tv,64,MILHAO,10); //o programa nao suporta tantas threads
	
	for(int i = 0; i < tv.size; i++)
	{
		teste(tv.tests[i]);//realiza testes
		printf("\n");
	}
	
	pthread_mutex_destroy(&mutex);
	printf("FIM\n");
	return 0;
}





