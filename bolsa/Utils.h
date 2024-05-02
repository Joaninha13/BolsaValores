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
#define TAM					100
#define MAX					256
#define MAX_USERS			20
#define MAX_EMPRESAS		30
#define MAX_ACOES_BOARD		10


typedef struct {

	TCHAR userName[TAM], password[TAM];
	DWORD saldo;
	BOOL ativo; //TRUE se o usu�rio est� ativo, FALSE caso contr�rio

} User;
typedef struct {

	TCHAR name[TAM];
	DWORD numAcoes[MAX_USERS], valor; // numAcoes[i] � o n�mero de a��es que o usu�rio i tem. 
	User usersVenda[MAX_USERS]; // usersVenda[i] � o usu�rio que est� vendendo as a��es numAcoes[i]. o i = 0 � o unico null

} CompanyShares;


// Estrutura para opera��es de compra/venda
typedef struct {
	BOOL isCompra; // TRUE para compra, FALSE para venda
	TCHAR nomeEmpresa[TAM];
	DWORD quantidadeAcoes;
} Operation;
// Estrutura para responder/mandar mensagens ao cliente
typedef struct {

	User user; //Usuario a quem pertence a mensagem
	Operation operacao; // Opera��o atual (compra/venda)
	TCHAR mensagem[TAM]; // Mensagem de resposta da bolsa
	BOOL sucesso; // TRUE se a opera��o foi bem sucedida, FALSE caso contr�rio
	HANDLE hPipe; // Handle do pipe para responder ao cliente

} Response;

typedef struct {

	//TopAcoes decrescente

	CompanyShares topAcoes[MAX_EMPRESAS];
	CompanyShares venda;
	BOOL isCompra; // TRUE para compra, FALSE para venda

} MemoryShare;


typedef struct {

	HANDLE	hPipes[NCLIENTES],	// Handles dos pipes dos clientes
			hMutex, // Mutex para proteger a mem�ria compartilhada
			trinco, // Trinco para proteger o pipeData
			hEvent,	// Evento para sinalizar que a mem�ria compartilhada foi atualizada
			hMap,	// Mapeamento da mem�ria compartilhada
			hSem;	// Sem�foro para controlar o numero de clientes para a cria��o de pipes

	User users[MAX_USERS];
	CompanyShares company[MAX_EMPRESAS];

	BOOL continua;

	MemoryShare *memory;

} BolsaThreads;

typedef struct {

	Operation operacao;
	Response resp;

	BolsaThreads* bolsaData; // Ponteiro para a estrutura BolsaThreads � preciso para aceder aos hPipes[Nclientes]

	HANDLE hMutex, hPipe;
	DWORD id;

} PipeData;
