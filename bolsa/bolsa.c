#include "Utils.h"


BOOL leComand(TCHAR comand[TAM_COMAND], BolsaThreads* data) {

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
	if (_tcscmp(first, _T("addcf")) == 0) {
		//Acrescentar uma empresa por um ficheiro existente
	}

	else if (_tcscmp(first, _T("listc")) == 0) {
		//Lista todas as empresas existentes na bolsa, mostrando o seu nome, número de ações disponíveis (remanescentes), e preço atual das ações.

		//talvez por aqui um mutex para protejer.

		for (DWORD i = 0; i < MAX_EMPRESAS; i++){
			if (_tcscmp(data->company[i].name, _T("")) != 0)
				_tprintf(_T("Empresa %d: %s - Valor por ação : %d - Número de ações : %d\n"), i + 1, data->company[i].name, data->company[i].valor, data->company[i].numAcoes[0]);
		}
	}

	else if (_tcscmp(first, _T("stock")) == 0){
		//mudar o preço de uma ação

		_tprintf(_T("Empresa %s\n"), secon);


		for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
			if (_tcscmp(data->company[i].name, secon) == 0) {
				data->company[i].valor = _tstoi(third);
				_tprintf(_T("O valor da ação da empresa %s foi alterado para %d\n"), data->company[i].name, data->company[i].valor);
				break;
			}
		}
	}

	else if (_tcscmp(first, _T("users")) == 0) {
		//Permite listar todos os utilizadores registados, mostrando o seu username, saldo atual e estado (ativo ou inativo).

		for (DWORD i = 0; i < MAX_USERS; i++) {
			if (_tcscmp(data->users[i].userName, _T("")) != 0)
				_tprintf(_T("Utilizador %d: %s - Saldo : %d - Estado : %s\n"), i + 1, data->users[i].userName, data->users[i].saldo, data->users[i].ativo ? _T("Ativo") : _T("Inativo"));
		}

	}

	else if (_tcscmp(first, _T("pause")) == 0) {
		//Este comando faz com que as operações de compra e venda sejam suspensas (ignoradas) durante um período de tempo.Qualquer pedido de compra e venda que surja nesse período não será concretizado.
	}

	else if (_tcscmp(first, _T("close")) == 0) {
		//Permite encerrar o sistema. Todos os clientes e boards serão notificados, devendo terminar de seguida. 
	}

	return TRUE;
}

BOOL InicializaAll(BolsaThreads* all) {

	DWORD NClientes_Reg = 0;
	HANDLE hMapFile;
	HKEY hKey;

	//Verificar se o ficheiro de memoria partilhada ja existe, se existir, entao mandar uma mensagema  dizer que ja existe um server, se nao existir, entao criar
	//Done

	hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, FILE_MAPPING_NAME);
		if (hMapFile != NULL) {
			_tprintf(_T("Já existe um servidor em execução.\n"));
			return FALSE;
		}

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

	//Ir buscar o NClientes ao Reg., se nao houver ja a key apresentada no REGISTRYPATH, entao criar a key com um valor de NClientes_Reg = NCLIENTES
	//Done
	if (RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRYPATH, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		DWORD dwSize = sizeof(DWORD);
		if (RegQueryValueEx(hKey, _T("NClientes"), NULL, NULL, (LPBYTE)&NClientes_Reg, &dwSize) != ERROR_SUCCESS) {
			_tprintf(_T("Error : RegQueryValueEx (%d)\n"), GetLastError());
			return FALSE;
		}

		_tprintf(_T("NClientes = %d\n"), NClientes_Reg);
	}
	else {
		if (RegCreateKeyEx(HKEY_CURRENT_USER, REGISTRYPATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
			_tprintf(_T("Error : RegCreateKeyEx (%d)\n"), GetLastError());
			return FALSE;
		}
		NClientes_Reg = NCLIENTES;
		if (RegSetValueEx(hKey, _T("NClientes"), 0, REG_DWORD, (LPBYTE)&NClientes_Reg, sizeof(DWORD)) != ERROR_SUCCESS) {
			_tprintf(_T("Error : RegSetValueEx (%d)\n"), GetLastError());
			return FALSE;
		}
	}


	//Para ser utilizado no controlo da Criação de Named Pipes e entrada/saida de clientes
	all->hSem = CreateSemaphore(NULL, NClientes_Reg,NClientes_Reg, SEMAPHORE_NAME);
		if (all->hSem == NULL) {
			_tprintf(_T("Error : CreateSemaphore (%d)\n"), GetLastError());
			return FALSE;
		}

	all->hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);
		if (all->hMutex == NULL) {
			_tprintf(_T("Error : CreateMutex (%d)\n"), GetLastError());
			return FALSE;
		}


	//Criar o evento que vai ser usado para dizer ao board que ah updates na memoria partilhada
	all->hEvent = CreateEvent(NULL, TRUE, FALSE, EVENT_NAME);
		if (all->hEvent == NULL) {
			_tprintf(_T("Error : CreateEvent (%d)\n"), GetLastError());
			return FALSE;
		}

	
	all->continua = TRUE;
	
	return TRUE;
}

BOOL testarMemoria(BolsaThreads* data) {

	MemoryShare dataMem;

	dataMem.isCompra = FALSE;
	dataMem.venda.numAcoes[0] = 100;
	dataMem.venda.valor = 50;


	WaitForSingleObject(data->hMutex, INFINITE);

	//CopyMemory(data->memory, &dataMem, sizeof(MemoryShare));

	data->memory->isCompra = FALSE;
	data->memory->venda.numAcoes[0] = 100;
	data->memory->venda.valor = 50;

	ReleaseMutex(data->hMutex);

	SetEvent(data->hEvent);

	ResetEvent(data->hEvent);

	return TRUE;

}


//acabar estas duas funcoes
DWORD WINAPI trataCliente(LPVOID data) {

	PipeData* pdata = (PipeData*)data;
	HANDLE hPipe;
	DWORD nBytes;
	BOOL rSuccess, wSuccess;

	OVERLAPPED ov;
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); //Evento para avisar que ja leu....

	WaitForSingleObject(pdata->bolsaData->trinco, INFINITE);
	hPipe = pdata->bolsaData->hPipes[pdata->id];
	ReleaseMutex(pdata->bolsaData->trinco);

	do {
		ZeroMemory(&ov, sizeof(OVERLAPPED));
		ov.hEvent = hEvent;

		rSuccess = ReadFile(hPipe, &pdata->operacao, sizeof(Operation), &nBytes, &ov);

		if (!rSuccess) {
			if (GetLastError() == ERROR_IO_PENDING) {
				_tprintf(_T("Agendei uma leitura no cliente %d\n"), pdata->id);

				WaitForSingleObject(hEvent, INFINITE);

				rSuccess = GetOverlappedResult(hPipe, &ov, &nBytes, FALSE);
			}
			else {
				_tprintf(_T("[ERRO] Ler do pipe do cliente %d"), pdata->id);
				break;
			}
		}





	} while (1);


	return 0;

}
DWORD WINAPI escreveCli() {






	return 0;
}



DWORD WINAPI createPipe(LPVOID data) {

	BolsaThreads* pdata = (BolsaThreads*)data;
	PipeData dataPipe[NCLIENTES];


	HANDLE hPipe, hThreads[NCLIENTES];

	pdata->trinco = CreateMutex(NULL, FALSE, NULL);
	if (pdata->trinco == NULL) {
		_tprintf(_T("[ERRO] Criar Mutex no createPipe! (CreateMutex)"));
		exit(-1);
	}

	for (DWORD i = 0; i < NCLIENTES; i++)
		pdata->hPipes[i] = NULL;

	do {

		WaitForSingleObject(pdata->hSem, INFINITE);

		//Criar o pipe para o cliente
		_tprintf(_T("[SERVIDOR] Criar a instancia do pipe '%s' ... (CreateNamedPipe)\n"), PIPE_NAME);

		hPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, PIPE_UNLIMITED_INSTANCES, sizeof(PipeData), sizeof(PipeData), 1000, NULL);
		if (hPipe == INVALID_HANDLE_VALUE) {
			_tprintf(_T("[ERRO] Criar Named Pipe! (CreateNamedPipe)"));
			exit(-1);
		}

		//Esperar que o cliente se conecte
		_tprintf(_T("[SERVIDOR] Esperar que o cliente se conecte ... (ConnectNamedPipe)\n"));

		if (!ConnectNamedPipe(hPipe, NULL)) {
			_tprintf(_T("[ERRO] Conectar Named Pipe! (ConnectNamedPipe)"));
			exit(-1);
		}


		WaitForSingleObject(pdata->trinco, INFINITE);

		for (DWORD i = 0; i < NCLIENTES; i++) {
			if (pdata->hPipes[i] == NULL) {
				pdata->hPipes[i] = hPipe;
				dataPipe[i].id = i;

				hThreads[i] = CreateThread(NULL, 0, trataCliente, (LPVOID)&dataPipe[i], 0, NULL);
				if (hThreads[i] == NULL) {
					_tprintf(_T("[ERRO] Criar a thread! (CreateThread)\n"));
					exit(-1);
				}

				break;
			}
		}

		ReleaseMutex(pdata->trinco);


	} while (pdata->continua);


	for (int i = 0; i < NCLIENTES; i++) {
		if (pdata->hPipes[i] != NULL) {
			WaitForSingleObject(hThreads[i], INFINITE);
			CloseHandle(hThreads[i]);
		}

	}

	return 0;

}


int _tmain(int argc, TCHAR* argv[]) {

	TCHAR comand[TAM_COMAND];

	BolsaThreads dataThreads;

	HANDLE hThread[5];

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif 




	/*if (argc != 2) {
		_tprintf(_T("Usage: %s <file_name.txt>\n"), argv[0]);
		return 1;
	}*/

	if (!InicializaAll(&dataThreads))
		exit(1);

	//Criar a thread que vai tratar da criação dos Named Pipes e das threads para cada cliente.
	hThread[0] = CreateThread(NULL, 0, createPipe, (LPVOID)&dataThreads, 0, NULL);

	//Criar a thread que vai atualizar a memoria partilhada
	//hThread[0] = CreateThread(NULL, 0, AtualizaMemoria, &dataThreads, 0, NULL);


	//hThread = CreateThread(NULL, )

	//Era para testar se a escrita na memoria partilhada estava a funcionar, e esta a dar e tem que ser assim
	//Sleep(10000);
	//testarMemoria(&dataThreads);

	ZeroMemory(&dataThreads, sizeof(BolsaThreads));


	//para testes
	// 
	//for (int i = 0; i < 5; i++) {
	//	swprintf_s(dataThreads.company[i].name, TAM, _T("empresa%d"), i + 1);

	//	dataThreads.company[i].valor = 100 + i * 10;
	//	dataThreads.company[i].numAcoes[0] = 1000 + i * 100;
	//}

	//for (int i = 0; i < 5; i++) {
	//	swprintf_s(dataThreads.users[i].userName, TAM, _T("user%d"), i + 1);

	//	dataThreads.users[i].saldo = 1000 + i * 100;
	//	dataThreads.users[i].ativo = TRUE;
	//

	//}

	//swprintf_s(dataThreads.users[5].userName, TAM, _T("user%d"), 5 + 1);

	//dataThreads.users[5].saldo = 1000 + 5 * 100;
	//dataThreads.users[5].ativo = FALSE;


	do {

		_tprintf(_T("\n > "));
		_fgetts(comand, TAM_COMAND, stdin);
		comand[_tcslen(comand) - 1] = '\0'; //Retirar o \n
		leComand(comand, &dataThreads);

	} while (_tcscmp(comand, _T("close")) != 0);

    //avisar todos os clientes que o servidor está fechando e fechar as coisas que existem abertas

	return 0;
}