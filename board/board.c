#include "utils.h"

BOOL InicializaAll(TData* all) {

	all->continua = TRUE;

	all->hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, FILE_MAPPING_NAME);
	if (all->hMap == NULL) {
		_tprintf(_T("servidor ainda nao se encontra em execução.\n"));
		return FALSE;
	}

	all->shared = (MemoryShare*)MapViewOfFile(all->hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(MemoryShare));
	if (all->shared == NULL) {
		_tprintf(_T("Error : MapViewOfFile (%d)\n"), GetLastError());
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

	return TRUE;
}

void ordenarDecrescente(MemoryShare * s, int tamanho) {
	int i, j, max_index;


	CompanyShares temp;

	for (i = 0; i < tamanho - 1; i++) {
		max_index = i;

		for (j = i + 1; j < tamanho; j++)
			if (s->topAcoes[j].valor > s->topAcoes[max_index].valor)
				max_index = j;
		


		temp = s->topAcoes[i];
		s->topAcoes[i] = s->topAcoes[max_index];
		s->topAcoes[max_index] = temp;
	}
}

DWORD WINAPI actualizaMostra(LPVOID dados) {

	TData* td = (TData*)dados;

	MemoryShare shared;

	ZeroMemory(&shared, sizeof(MemoryShare));;


	do {

		WaitForSingleObject(td->hEvent, INFINITE);

		WaitForSingleObject(td->hMutex, INFINITE);

		CopyMemory(&shared, td->shared, sizeof(MemoryShare));

		ReleaseMutex(td->hMutex);

		ordenarDecrescente(&shared, MAX_EMPRESAS);

		for (DWORD i = 0; i < td->numEmpresas; i++)
			//if (_tcscmp(shared.topAcoes[i].name, _T("") != 0))
				_tprintf_s(_T("%d. %s - Valor da ação: %.2f\n"), i + 1, shared.topAcoes[i].name, shared.topAcoes[i].valor);


		if (shared.isCompra && shared.venda.name != NULL)
			_tprintf_s(_T("Compra\n"));
		else
			_tprintf_s(_T("Posto a Venda\n"));

		_tprintf_s(_T("Empresa - %s\n"), shared.venda.name);
		_tprintf_s(_T("Numero de Acções - %d\n"), shared.venda.numAcoes);
		_tprintf_s(_T("Valor - %.2f\n"), shared.venda.valor);

		//for (int i = 0; i < td->numEmpresas; i++)
		//	//if (_tcscmp(shared.topAcoes[i].name, _T("") != 0))
		//	_tprintf_s(_T("%d. %s - Valor da ação: %.2f\n"), i + 1, td->shared->topAcoes[i].name, td->shared->topAcoes[i].valor);


	} while (td->shared->continua || td->continua);


	return 0;
}


int _tmain(int argc, TCHAR* argv[]) {


	DWORD nEmpresas = 0;
	TData dataThread;
	TCHAR buffer[100];
	HANDLE hThread;


#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif 

	if (argc != 2) {
		_tprintf(_T("Usage: %s <N_Empresas>\n"), argv[0]);
		return 1;
	}

	ZeroMemory(&dataThread, sizeof(TData));

	if (!InicializaAll(&dataThread))
		exit(1);


	dataThread.numEmpresas = _tstoi(argv[1]);


	hThread = CreateThread(NULL, 0, actualizaMostra, &dataThread, 0, NULL);


	do{

		_tprintf_s(_T("1 - Escreva 'leave' para sair!\n"));
		_fgetts(buffer, 100, stdin);
		buffer[_tcslen(buffer) - 1] = '\0'; //Retirar o \n

	} while (_tcscmp(buffer, _T("leave")) != 0 || dataThread.shared->continua);

	dataThread.continua = FALSE;


	return 0;
}