#pragma once // Utils da bolsa

#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <string.h>

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

	TCHAR empresa[TAM];
	DWORD numAcoes[MAX_USERS], valor; // numAcoes[i] � o n�mero de a��es que o usu�rio i tem.
	User usersVenda[MAX_USERS]; // usersVenda[i] � o usu�rio que est� vendendo as a��es numAcoes[i].

} Empresas;


typedef struct {

	User users[MAX_USERS];
	Empresas empresas[MAX_EMPRESAS];

} Bolsa;


typedef struct {

	Empresas acoes[MAX_ACOES_BOARD];
	Empresas venda;

} MemoriaPartilhadaBoard;




