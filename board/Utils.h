#pragma once // Utils do board

//#include <windows.h>
//#include <tchar.h>
//#include <math.h>
//#include <stdio.h>
//#include <fcntl.h> 
//#include <io.h>
//#include <string.h>
//
//#define FILE_MAPPING_NAME	_T("FILE_SHARE")
//#define EVENT_NAME			_T("SHARE_EVENT_BOARD")
//#define MUTEX_NAME			_T("MUTEX_DATA")
//
//#define TAM				256
//#define MAX_USERS		20
//#define MAX_ACOES_BOARD 10
//#define MAX_EMPRESAS	30
//
//typedef struct {
//
//	TCHAR userName[TAM], password[TAM];
//	float saldo;
//	BOOL ativo; //TRUE se o usu�rio est� ativo, FALSE caso contr�rio
//	HANDLE hPipe; // Pipe do cliente
//
//} User;
//typedef struct {
//
//	TCHAR name[TAM];
//	DWORD numAcoes[MAX_USERS]; // numAcoes[i] � o n�mero de a��es que o usu�rio i tem. 
//	double valor;
//	User usersVenda[MAX_USERS]; // usersVenda[i] � o usu�rio que est� vendendo as a��es numAcoes[i]. o i = 0 � o unico null
//
//} CompanyShares;
//
//typedef struct {
//	TCHAR name[TAM];
//	DWORD numAcoes;
//	double valor;
//} ListCompany;
//
//typedef struct {
//
//	CompanyShares topAcoes[MAX_EMPRESAS];
//	ListCompany venda;
//	BOOL isCompra; // TRUE para compra, FALSE para venda
//
//	BOOL continua;
//
//} MemoryShare;

#include "../bolsa/Utils.h";

typedef struct {

	MemoryShare *shared;
	HANDLE hEvent, hMutex, hMap;
	BOOL continua;
	DWORD numEmpresas;

}TData;

