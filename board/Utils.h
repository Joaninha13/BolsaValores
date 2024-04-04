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
	DWORD numAcoes[MAX_USERS], valor; // numAcoes[i] é o número de ações que o usuário i tem.
	User usersVenda[MAX_USERS]; // usersVenda[i] é o usuário que está vendendo as ações numAcoes[i].

} Empresas;


typedef struct {

	Empresas acoes[MAX_ACOES_BOARD];
	Empresas venda;

} MemoriaPartilhadaBoard;

