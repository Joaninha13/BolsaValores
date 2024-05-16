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
#include <time.h>

#define PIPE_NAME _T("\\\\.\\pipe\\bolsa")


#define MAX_EMPRESAS 30
#define MAX_NOME 50
#define TAM 256


// Estrutura para operações de compra/venda
typedef struct {
    BOOL isCompra; // TRUE para compra, FALSE para venda
    TCHAR nomeEmpresa[TAM];
    DWORD quantidadeAcoes;
} Operation;

typedef struct {
    TCHAR name[TAM];
    DWORD numAcoes;
    DWORD valor;
} ListCompany;
// Estrutura para res

// Estrutura para responder/mandar mensagens ao cliente
typedef struct {

    //User user; //Usuario a quem pertence a mensagem

    TCHAR mensagem[TAM]; // Mensagem de resposta da bolsa
    BOOL sucesso; // TRUE se a operação foi bem sucedida, FALSE caso contrário
    Operation operacao; // Operação atual (compra/venda)
    ListCompany listCompany[MAX_EMPRESAS];

    //HANDLE hPipe; // Handle do pipe para responder ao cliente

} Response;

// Estrutura principal do cliente
typedef struct {
    Operation operacao; // Operação atual (compra/venda)
    Response resposta; // Resposta da última operação
} Cliente;