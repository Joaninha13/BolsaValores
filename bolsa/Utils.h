#pragma once // Utils da bolsa

#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <string.h>

#define REGISTRYPATH	_T("Software\\TPSO2\\Clientes")

#define PIPE_NAME		_T("\\\\.\\pipe\\bolsa")


#define FILE_MAPPING_NAME	_T("FILE_SHARE")
#define EVENT_NAME			_T("SHARE_EVENT_BOARD")
#define MUTEX_NAME			_T("MUTEX_DATA")
#define SEMAPHORE_NAME		_T("SEMAPHORE")


#define NCLIENTES			5
#define TAM_COMAND			250
#define TAM					256
#define MAX					256
#define MAX_USERS			20
#define MAX_EMPRESAS		30
#define MAX_ACOES_BOARD		10
#define MAX_ACOES_USER		5


typedef struct {

	TCHAR userName[TAM], password[TAM];
	float saldo;
	BOOL ativo; //TRUE se o usuário está ativo, FALSE caso contrário
	HANDLE hPipe; // Pipe do cliente

} User;
typedef struct {

	TCHAR name[TAM];
	DWORD numAcoes[MAX_USERS]; // numAcoes[i] é o número de ações que o usuário i tem. 
	double valor;
	User usersVenda[MAX_USERS]; // usersVenda[i] é o usuário que está vendendo as ações numAcoes[i]. o i = 0 é o unico null

} CompanyShares;

typedef struct {
	TCHAR name[TAM];
	DWORD numAcoes;
	double valor;
} ListCompany;

// Estrutura para a carteira de ações do cliente
typedef struct {
	ListCompany acoes[MAX_ACOES_USER]; // Ações do usuário
	int numEmpresas; // Número de empresas diferentes na carteira
} Wallet;


// Estrutura para operações de compra/venda
typedef struct {
	BOOL isCompra; // TRUE para compra, FALSE para venda
	TCHAR nomeEmpresa[TAM];
	DWORD quantidadeAcoes;
} Operation;

// Estrutura para responder/mandar mensagens ao cliente
typedef struct {

	//User user; //Usuario a quem pertence a mensagem

	TCHAR mensagem[TAM]; // Mensagem de resposta da bolsa
	BOOL sucesso; // TRUE se a operação foi bem sucedida, FALSE caso contrário
	Operation operacao; // Operação atual (compra/venda)
	ListCompany listCompany[MAX_EMPRESAS];

	//HANDLE hPipe; // Handle do pipe para responder ao cliente

} Response;

typedef struct {

	CompanyShares topAcoes[MAX_EMPRESAS];
	ListCompany venda;
	BOOL isCompra; // TRUE para compra, FALSE para venda

} MemoryShare;


typedef struct {

	HANDLE	hPipes[NCLIENTES],	// Handles dos pipes dos clientes
			hMutex, // Mutex para proteger a memória compartilhada
			trinco, // Trinco para proteger o pipeData
			hMutexData, // Mutex para proteger todos os dados da bolsa
			hEvent,	// Evento para sinalizar que a memória compartilhada foi atualizada
			hMap,	// Mapeamento da memória compartilhada
			hSem;	// Semáforo para controlar o numero de clientes para a criação de pipes

	User users[MAX_USERS];
	CompanyShares company[MAX_EMPRESAS];

	BOOL continua;

	MemoryShare *memory;

} BolsaThreads;

typedef struct {

	Response resp;

	BolsaThreads* bolsaData; // Ponteiro para a estrutura BolsaThreads é preciso para aceder aos hPipes[Nclientes]

	HANDLE hPipe;
	BOOL continua;
	DWORD id;

} PipeData;
