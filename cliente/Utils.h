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

// Estrutura para autenticação do cliente
typedef struct {
    TCHAR username[TAM];
    TCHAR password[TAM];
    DWORD saldo; 
} User;

// Estrutura para representar ações de uma empresa 
typedef struct {
    TCHAR nomeEmpresa[MAX_NOME];
    DWORD quantidadeAcoes;
    float precoAcao; // O preço por ação
} CompanyShares;

// Estrutura para a carteira de ações do cliente
typedef struct {
    CompanyShares acoes[MAX_EMPRESAS];
    int numEmpresas; // Número de empresas diferentes na carteira
} Wallet;

// Estrutura para operações de compra/venda
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
    User user; // Informações do usuário
    Wallet wallet; // Carteira de ações do usuário
    Operation operacao; // Operação atual (compra/venda)
    Response resposta; // Resposta da última operação
} Cliente;