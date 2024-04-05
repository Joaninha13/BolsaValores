#pragma once //Utils do Cliente

#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <process.h>
#include <threads.h>
#include <time.h>

#define MAX_EMPRESAS 30
#define MAX_NOME 50
#define TAM 100

// Estrutura para autentica��o do cliente
typedef struct {
    TCHAR username[TAM];
    TCHAR password[TAM];
    DWORD saldo; 
} User;

// Estrutura para representar a��es de uma empresa 
typedef struct {
    TCHAR nomeEmpresa[MAX_NOME];
    DWORD quantidadeAcoes;
    float precoAcao; // O pre�o por a��o
} CompanyShares;

// Estrutura para a carteira de a��es do cliente
typedef struct {
    CompanyShares acoes[MAX_EMPRESAS];
    int numEmpresas; // N�mero de empresas diferentes na carteira
} Wallet;

// Estrutura para opera��es de compra/venda
typedef struct {
    BOOL isCompra; // TRUE para compra, FALSE para venda
    TCHAR nomeEmpresa[MAX_NOME];
    DWORD quantidadeAcoes;
} Operation;

// Estrutura para armazenar as respostas da bolsa
typedef struct {
    TCHAR mensagem[MAX_NOME]; // Mensagem de resposta da bolsa
    BOOL sucesso; 
} Response;

// Estrutura principal do cliente
typedef struct {
    User user; // Informa��es do usu�rio
    Wallet wallet; // Carteira de a��es do usu�rio
    Operation operacao; // Opera��o atual (compra/venda)
    Response resposta; // Resposta da �ltima opera��o
} Cliente;