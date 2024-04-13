#pragma once // Utils da bolsa

#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <string.h>

#define REGISTRYPATH _T("Software\\TPSO2\\BolsaConfigNClients")

#define PIPE_NAME _T("\\\\.\\pipe\\bolsa")


#define FILE_MAPPING_NAME	_T("FILE_SHARE")
#define EVENT_NAME			_T("SHARE_EVENT")
#define MUTEX_NAME			_T("Mutex")
#define TAM_COMAND			250
#define TAM 100
#define MAX_USERS 20
#define MAX_EMPRESAS 30
#define MAX_ACOES_BOARD 10

// Vamos fazer estruturas de dados para um exemplo de bolsa de valores
// Cada usuário tem um nome, senha, saldo
// Cada ação para venda tem nome da empresa, o número de ações a vende o valor de cada ação e o usuário que está vendendo
// Cada empresa tem um nome, o numero de ações e um valor


typedef struct {

	TCHAR userName[TAM], password[TAM];
	DWORD saldo;

} User;
typedef struct {

	TCHAR company[TAM];
	DWORD numAcoes[MAX_USERS], valor; // numAcoes[i] é o número de ações que o usuário i tem.
	User usersVenda[MAX_USERS]; // usersVenda[i] é o usuário que está vendendo as ações numAcoes[i].

} CompanyShares;
// Estrutura para operações de compra/venda
typedef struct {
	BOOL isCompra; // TRUE para compra, FALSE para venda
	TCHAR nomeEmpresa[TAM];
	DWORD quantidadeAcoes;
} Operation;
// Estrutura para responder/mandar mensagens ao cliente
typedef struct {

	User user; //Usuario a quem pertence a mensagem
	Operation operacao; // Operação atual (compra/venda)
	TCHAR mensagem[TAM]; // Mensagem de resposta da bolsa
	BOOL sucesso; // TRUE se a operação foi bem sucedida, FALSE caso contrário
	HANDLE hPipe; // Handle do pipe para responder ao cliente

} Response;



typedef struct {

	HANDLE Hmutex, hEvent, hMap, hSem; // depois ver se é preciso mais alguma coisa

	User users[MAX_USERS];
	CompanyShares company[MAX_EMPRESAS];

	MemoryShare* memory;

} BolsaThreads;

typedef struct {

	CompanyShares acoes[MAX_ACOES_BOARD];
	CompanyShares venda;

} MemoryShare;

