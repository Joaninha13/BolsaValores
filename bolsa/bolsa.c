#include "utils.h"


BOOL leComand(TCHAR comand[TAM_COMAND]) {

	int auxi = 0;

	TCHAR first[TAM_COMAND], secon[TAM_COMAND], third[TAM_COMAND], four[TAM_COMAND];
	TCHAR* context = NULL;

	TCHAR* token = _tcstok_s((TCHAR*)comand, _T(" "), &context);

	while (token != NULL) {

		if (auxi == 0)
			_tcscpy_s(first, TAM_COMAND, token);
		else if (auxi == 1)
			_tcscpy_s(secon, TAM_COMAND, token);
		else if (auxi == 2)
			_tcscpy_s(third, TAM_COMAND, token);
		else
			_tcscpy_s(four, TAM_COMAND, token);

		auxi++;

		token = _tcstok_s(NULL, _T(" "), &context);
	}

	if (_tcscmp(first, _T("addc")) == 0) {
		//Acrescentar uma empresa

	}
	else if (_tcscmp(first, _T("listc")) == 0) {
		//Lista todas as empresas existentes na bolsa, mostrando o seu nome, número de ações disponíveis (remanescentes), e preço atual das ações.

	}
	else if (_tcscmp(first, _T("stock")) == 0){
		//mudar o preço de uma ação

	}
	else if (_tcscmp(first, _T("users")) == 0) {
		//Permite listar todos os utilizadores registados, mostrando o seu username, saldo atual e estado (ativo ou inativo).

	}
	else if (_tcscmp(first, _T("pause")) == 0) {
		//Este comando faz com que as operações de compra e venda sejam suspensas (ignoradas) durante um período de tempo.Qualquer pedido de compra e venda que surja nesse período não será concretizado.
	}
	else if (_tcscmp(first, _T("close")) == 0) {
		//Permite encerrar o sistema. Todos os clientes e boards serão notificados, devendo terminar de seguida. 
	}


}

BOOL InicializaAll(BolsaThreads* all) {

	all->hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(MemoryShare), FILE_MAPPING_NAME);
		if (all->hMap == NULL) {
			_tprintf(_T("Error : CreateFileMapping (%d)\n"), GetLastError());
			return FALSE;
		}

	all->memory = (MemoryShare*)MapViewOfFile(all->hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(MemoryShare));
		if (all->memory == NULL) {
			_tprintf(_T("Error : MapViewOfFile (%d)\n"), GetLastError());
			return FALSE;
		}



	
	return TRUE;
}

int _tmain(int argc, TCHAR* argv[]) {

	TCHAR comand[TAM_COMAND];

	BolsaThreads dataThreads;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif 




	if (argc != 2) {
		_tprintf(_T("Usage: %s <file_name.txt>\n"), argv[0]);
		return 1;
	}

	if (!InicializaAll(&dataThreads)) {
		_tprintf(_T("Error creating sh memory and sync"));
		exit(1);
	}



	do {

		_fgetts(comand, TAM_COMAND, stdin);
		comand[_tcslen(comand) - 1] = '\0'; //Retirar o \n

	} while (_tcscmp(comand, _T("close")) != 0);

    //avisar todos os clientes que o servidor está fechando e fechar as coisas que existem abertas

	return 0;
}