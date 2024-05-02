#pragma once // Utils do board

#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <string.h>

#define FILE_MAPPING_NAME	_T("FILE_SHARE")
#define EVENT_NAME			_T("SHARE_EVENT_BOARD")
#define MUTEX_NAME			_T("MUTEX_DATA")

#define TAM				100
#define MAX_USERS		20
#define MAX_ACOES_BOARD 10
#define MAX_EMPRESAS	30

typedef struct {

	TCHAR userName[TAM], password[TAM];
	DWORD saldo;

} User;

typedef struct {

	TCHAR empresa[TAM];
	DWORD numAcoes[MAX_USERS], valor; // numAcoes[i] é o número de ações que o usuário i tem.
	User usersVenda[MAX_USERS]; // usersVenda[i] é o usuário que está vendendo as ações numAcoes[i].

} CompanyShares;


typedef struct {

	//TopAcoes decrescente

	CompanyShares topAcoes[MAX_EMPRESAS];
	CompanyShares venda;
	BOOL isCompra; // TRUE para compra, FALSE para venda

} MemoryShare;


typedef struct {

	MemoryShare *shared;
	HANDLE hEvent, hMutex, hMap;
	BOOL continua;
	DWORD numEmpresas;

}TData;

