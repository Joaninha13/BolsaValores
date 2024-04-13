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
// Cada usu�rio tem um nome, senha, saldo
// Cada a��o para venda tem nome da empresa, o n�mero de a��es a vende o valor de cada a��o e o usu�rio que est� vendendo
// Cada empresa tem um nome, o numero de a��es e um valor


typedef struct {

	TCHAR userName[TAM], password[TAM];
	DWORD saldo;

} User;
typedef struct {

	TCHAR company[TAM];
	DWORD numAcoes[MAX_USERS], valor; // numAcoes[i] � o n�mero de a��es que o usu�rio i tem.
	User usersVenda[MAX_USERS]; // usersVenda[i] � o usu�rio que est� vendendo as a��es numAcoes[i].

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

	HANDLE Hmutex, hEvent, hMap, hSem; // depois ver se � preciso mais alguma coisa

	User users[MAX_USERS];
	CompanyShares company[MAX_EMPRESAS];

	MemoryShare* memory;

} BolsaThreads;

typedef struct {

	CompanyShares acoes[MAX_ACOES_BOARD];
	CompanyShares venda;

} MemoryShare;

