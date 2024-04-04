#pragma once // Utils do board

#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <string.h>

#define TAM 100
#define MAX_USERS 20
#define MAX_ACOES_BOARD 10

typedef struct {

	TCHAR userName[TAM], password[TAM];
	DWORD saldo;

} User;

typedef struct {

	TCHAR empresa[TAM];
	DWORD numAcoes[MAX_USERS], valor; // numAcoes[i] � o n�mero de a��es que o usu�rio i tem.
	User usersVenda[MAX_USERS]; // usersVenda[i] � o usu�rio que est� vendendo as a��es numAcoes[i].

} Empresas;


typedef struct {

	Empresas acoes[MAX_ACOES_BOARD];
	Empresas venda;

} MemoriaPartilhadaBoard;

