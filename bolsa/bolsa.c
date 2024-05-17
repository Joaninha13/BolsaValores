#include "Utils.h"

//DONE
BOOL lerFicheiroUsers(TCHAR* nomeArquivo, User* usuario) {

	FILE* arquivo;
	if (_wfopen_s(&arquivo, nomeArquivo, _T("r")) != 0) {
		_tprintf(_T("Erro ao abrir o arquivo.\n"));
		return FALSE;
	}

	TCHAR linha[256]; // Buffer para armazenar a linha do arquivo
	int i = 0;

	// Lê e processa cada linha do arquivo até o final
	while (_fgetts(linha, sizeof(linha) / sizeof(TCHAR), arquivo) != NULL) {

		// Extrai os dados da linha
		TCHAR* nextToken = NULL;
		TCHAR* token = _tcstok_s(linha, _T(" "), &nextToken);


		if (token != NULL) {
			_tcscpy_s(usuario[i].userName, sizeof(usuario[i].userName), token); // Copia o primeiro token para userName
			token = _tcstok_s(NULL, _T(" "), &nextToken); // Avança para o próximo token
		}

		if (token != NULL) {
			_tcscpy_s(usuario[i].password, sizeof(usuario[i].password), token); // Copia o segundo token para password
			token = _tcstok_s(NULL, _T(" "), &nextToken);
		}

		if (token != NULL) {
			usuario[i].saldo = _tstof(token);; // Converte o terceiro token para double e poe na variavel saldo
		}

		usuario[i].ativo = FALSE; // Inicializa o usuário como offline
		usuario[i].hPipe = NULL; // Inicializa o número de ações como 0

		i++;
	}

	// Fecha o arquivo após a leitura
	fclose(arquivo);

	return TRUE;
}
//DONE
BOOL lerFicheiroCompanys(TCHAR* nomeArquivo, CompanyShares* comp) {

	int i = 0;

	FILE* arquivo;
	if (_wfopen_s(&arquivo, nomeArquivo, _T("r")) != 0) {
		_tprintf(_T("Erro ao abrir o arquivo.\n"));
		return FALSE;
	}

	TCHAR linha[256]; // Buffer para armazenar a linha do arquivo

	for (int j = 0; j < MAX_EMPRESAS; j++)
		if (_tcscmp(comp[j].name, _T("")) != 0)
			i = j;

	if (i != 0)
		i++;

	// Lê e processa cada linha do arquivo até o final
	while (_fgetts(linha, sizeof(linha) / sizeof(TCHAR), arquivo) != NULL) {

		// Extrai os dados da linha
		TCHAR* nextToken = NULL;
		TCHAR* token = _tcstok_s(linha, _T(" "), &nextToken);


		if (token != NULL) {
			_tcscpy_s(comp[i].name, sizeof(comp[i].name), token);
			_tcscpy_s(comp[i].usersVenda[0].userName, TAM, _T(""));
			token = _tcstok_s(NULL, _T(" "), &nextToken);
		}

		if (token != NULL) {
			comp[i].numAcoes[0] = _tstoi(token);
			token = _tcstok_s(NULL, _T(" "), &nextToken);
		}

		if (token != NULL) {
			comp[i].valor = _tstof(token);
		}

		for (int j = 1; j < MAX_USERS; j++)
			comp[i].numAcoes[j] = 0;

		i++;
	}

	// Fecha o arquivo após a leitura
	fclose(arquivo);

	return TRUE;
}

BOOL escreveCli(HANDLE* hPipes, Response *resp) {


	DWORD nBytes;
	OVERLAPPED ov;
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); //Evento para avisar que ja leu....

	ZeroMemory(&ov, sizeof(OVERLAPPED));

	ov.hEvent = hEvent;

	_tprintf(_T("Entrei no EscreveCli e vou escrever para todos -> %s\n"), resp->mensagem);

	for (int i = 0; i < NCLIENTES; i++) {

		if (hPipes[i] != NULL) {

			if (!WriteFile(hPipes[i], resp, sizeof(Response), &nBytes, NULL)) {

				if (GetLastError() == ERROR_IO_PENDING) {
					_tprintf(_T("Escrita agendada no cliente %d com sucesso enviados %d\n"), i, nBytes);

					WaitForSingleObject(hEvent, INFINITE);

					if (GetOverlappedResult(hPipes, &ov, &nBytes, FALSE)) {
						_tprintf(_T("Escrita no cliente com sucesso\n"));
					}
					else {
						_tprintf(_T("Erro na escrita no cliente\n"));
					}
				}
				else {
					_tprintf(_T("Erro na escrita no cliente\n"));
				}
			}
			else {
				_tprintf(_T("Escrita instantania no cliente %d com sucesso enviados %d\n"), i, nBytes);
			}

		}

	}

	return TRUE;
}


DWORD WINAPI stop(LPVOID data) {

	BolsaThreads* all = (BolsaThreads*)data;


	WaitForSingleObject(all->hMutexData, INFINITE);

	all->stop = TRUE;

	ReleaseMutex(all->hMutexData);


	Sleep(all->nStop * 1000);


	WaitForSingleObject(all->hMutexData, INFINITE);

	all->continua = FALSE;

	ReleaseMutex(all->hMutexData);


	return 0;


}

BOOL leComand(TCHAR comand[TAM_COMAND], BolsaThreads* data) {

	int auxi = 0;
	Response resp;

	memset(&resp, 0, sizeof(Response));

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


	//Done
	if (_tcscmp(first, _T("addc")) == 0) {
		//Acrescentar uma empresa

		WaitForSingleObject(data->hMutexData, INFINITE);

		for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
			if (_tcscmp(data->company[i].name, _T("")) == 0) {
				_tcscpy_s(data->company[i].name, TAM, secon);
				data->company[i].numAcoes[0] = _tstoi(third);
				data->company[i].valor = _tstof(four);
				_tcscpy_s(data->company[i].usersVenda[0].userName, TAM, _T(""));

				for (DWORD j = 1; j < MAX_USERS; j++){
					data->company[i].numAcoes[j] = 0;
				}

				_tprintf(_T("%s adicionada com sucesso\n"), data->company[i].name);
				break;
			}
		}

		WaitForSingleObject(data->hMutex, INFINITE);
		CopyMemory(data->memory->topAcoes, &data->company, MAX_EMPRESAS * sizeof(CompanyShares));
		ReleaseMutex(data->hMutex);

		for (int i = 0; i < MAX_EMPRESAS; i++){
			if (_tcscmp(data->memory->topAcoes[i].name, _T("")) != 0)
				_tprintf(_T("Empresa %d: %s - Valor por ação : %.2f \n"), i + 1, data->memory->topAcoes[i].name, data->memory->topAcoes[i].valor);

		}

		SetEvent(data->hEvent);
		ResetEvent(data->hEvent);

		ReleaseMutex(data->hMutexData);
		
	}
	//Done
	else if (_tcscmp(first, _T("addcf")) == 0) {
		//Acrescentar uma empresa por um ficheiro existente
		WaitForSingleObject(data->hMutexData, INFINITE);

		if (lerFicheiroCompanys(secon, data->company))
			_tprintf(_T("Dados lidos com sucesso do ficheiro Empresas\n"));


		WaitForSingleObject(data->hMutex, INFINITE);
		CopyMemory(data->memory->topAcoes, &data->company, MAX_EMPRESAS * sizeof(CompanyShares));
		ReleaseMutex(data->hMutex);

		for (int i = 0; i < MAX_EMPRESAS; i++) {
			if (_tcscmp(data->memory->topAcoes[i].name, _T("")) != 0)
				_tprintf(_T("Empresa %d: %s - Valor por ação : %.2f \n"), i + 1, data->memory->topAcoes[i].name, data->memory->topAcoes[i].valor);

		}

		SetEvent(data->hEvent);
		ResetEvent(data->hEvent);

		ReleaseMutex(data->hMutexData);
	}
	//Done
	else if (_tcscmp(first, _T("listc")) == 0) {
		//Lista todas as empresas existentes na bolsa, mostrando o seu nome, número de ações disponíveis (remanescentes), e preço atual das ações.

		WaitForSingleObject(data->hMutexData, INFINITE);

		for (DWORD i = 0; i < MAX_EMPRESAS; i++){
			if (_tcscmp(data->company[i].name, _T("")) != 0)
				_tprintf(_T("Empresa %d: %s - Valor por ação : %.2f - Número de ações : %d\n"), i + 1, data->company[i].name, data->company[i].valor, data->company[i].numAcoes[0]);
		}

		ReleaseMutex(data->hMutexData);	
	}
	//Done
	else if (_tcscmp(first, _T("stock")) == 0){
		//mudar o preço de uma ação

		WaitForSingleObject(data->hMutexData, INFINITE);

		for (DWORD i = 0; i < MAX_EMPRESAS; i++) {

			if (_tcscmp(data->company[i].name, secon) == 0) {
				data->company[i].valor = _tstof(third);
				_tprintf(_T("O valor da ação da empresa %s foi alterado para %.2f\n"), data->company[i].name, data->company[i].valor);

				// por um valor na resposta com o nome da empresa e o valor novo

				_stprintf_s(&resp.mensagem, TAM, _T("O valor da ação da empresa %s foi alterado para %.2f\n"), data->company[i].name, data->company[i].valor);

				if (escreveCli(data->hPipes, &resp)){
					_tprintf(_T("Mensagem enviada com sucesso\n"));
				}
				else {
					_tprintf(_T("Erro ao enviar mensagem\n"));
				}

				break;
			}

		}

		WaitForSingleObject(data->hMutex, INFINITE);
		CopyMemory(data->memory->topAcoes, &data->company, MAX_EMPRESAS * sizeof(CompanyShares));
		ReleaseMutex(data->hMutex);

		SetEvent(data->hEvent);
		ResetEvent(data->hEvent);


		ReleaseMutex(data->hMutexData);
	}
	//Done
	else if (_tcscmp(first, _T("users")) == 0) {
		//Permite listar todos os utilizadores registados, mostrando o seu username, saldo atual e estado (ativo ou inativo).

		WaitForSingleObject(data->hMutexData, INFINITE);

		for (DWORD i = 0; i < MAX_USERS; i++) {
			if (_tcscmp(data->users[i].password, _T("")) != 0) {
				_tprintf(_T("Utilizador %d: %s - Saldo : %.2f - Estado : %s\n"), i + 1, data->users[i].userName, data->users[i].saldo, data->users[i].ativo ? _T("Ativo") : _T("Inativo"));
				ReleaseMutex(data->hMutexData);
			}else
				break;
		}

		ReleaseMutex(data->hMutexData);

	}

	else if (_tcscmp(first, _T("pause")) == 0) {
		//Este comando faz com que as operações de compra e venda sejam suspensas (ignoradas) durante um período de tempo.Qualquer pedido de compra e venda que surja nesse período não será concretizado.
		
		data->nStop = _tstoi(secon);

		HANDLE aux = CreateThread(NULL, 0, stop, data, 0, NULL);

		WaitForSingleObject(aux, INFINITE);

		CloseHandle(aux);
}
	
	else if (_tcscmp(first, _T("close")) == 0) {
		//Permite encerrar o sistema. Todos os clientes e boards serão notificados, devendo terminar de seguida. 

		_tcscpy_s(resp.mensagem, TAM, _T("close"));

		escreveCli(data->hPipes, &resp);

		WaitForSingleObject(data->hMutex, INFINITE);
		data->memory->continua = FALSE;
		ReleaseMutex(data->hMutex);

		SetEvent(data->hEvent);
		ResetEvent(data->hEvent);



	}

	else
		_tprintf(_T("Comando inválido\n"));

	return TRUE;
}

//DONE
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
		
	all->memory = (MemoryShare*)MapViewOfFile(all->hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(MemoryShare));
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
			_tprintf(_T("Error : CreateMutex Memoria (%d)\n"), GetLastError());
			return FALSE;
		}

	all->hMutexData = CreateMutex(NULL, FALSE, NULL);
		if (all->hMutexData == NULL) {
			_tprintf(_T("Error : CreateMutex Data (%d)\n"), GetLastError());
			return FALSE;
		}


	//Criar o evento que vai ser usado para dizer ao board que ah updates na memoria partilhada
	all->hEvent = CreateEvent(NULL, TRUE, FALSE, EVENT_NAME);
		if (all->hEvent == NULL) {
			_tprintf(_T("Error : CreateEvent (%d)\n"), GetLastError());
			return FALSE;
		}
	
	all->continua = TRUE;
	all->stop = FALSE;
	all->memory->continua = TRUE;
	
	return TRUE;
}


//acabar estas duas funcoes
DWORD WINAPI trataCliente(LPVOID data) {

	PipeData* pdata = (PipeData*)data;
	DWORD nBytes;
	BOOL rSuccess, wSuccess;
	Response resp;
	Wallet wallet;

	OVERLAPPED ov;
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); //Evento para avisar que ja leu....

	WaitForSingleObject(pdata->bolsaData->trinco, INFINITE);
	pdata->hPipe = pdata->bolsaData->hPipes[pdata->id];
	ReleaseMutex(pdata->bolsaData->trinco);

	memset(&wallet, 0, sizeof(Wallet));

	do {

		memset(&resp, 0, sizeof(Response));

		ZeroMemory(&ov, sizeof(OVERLAPPED));
		ov.hEvent = hEvent;

		//ZeroMemory(&resp, sizeof(Response));
		

		rSuccess = ReadFile(pdata->hPipe, &pdata->resp, sizeof(Response), &nBytes, &ov);	

		if (!rSuccess) {
			if (GetLastError() == ERROR_IO_PENDING) {
				_tprintf(_T("Agendei uma leitura no cliente %d\n"), pdata->id);

				WaitForSingleObject(hEvent, INFINITE);

				rSuccess = GetOverlappedResult(pdata->hPipe, &ov, &nBytes, FALSE); //ver se é preciso

				_tprintf(_T(" Recebi NBytes -> %d\n"), nBytes);


				_tprintf(_T("mensagem : %s\n"), pdata->resp.mensagem);
				_tprintf(_T("sucesso : %d\n"), pdata->resp.sucesso);
				_tprintf(_T("Empresa : %s\n"), pdata->resp.operacao.nomeEmpresa);
				_tprintf(_T("quantidade : %d\n"), pdata->resp.operacao.quantidadeAcoes);
				_tprintf(_T("mensagem : %d\n"), pdata->resp.operacao.isCompra);



				if (rSuccess && nBytes > 0 && _tcscmp(pdata->resp.mensagem, _T("")) != 0) {
					_tprintf(_T("Leitura do cliente %d com sucesso\n"), pdata->id);
					_tprintf(_T("Comando : %s\n"), pdata->resp.mensagem);

					TCHAR* context = NULL;

					TCHAR* token = _tcstok_s(pdata->resp.mensagem, _T(" "), &context);

					//Done probably...
					if (_tcscmp(token, _T("login")) == 0) {
						//login username password

						_tprintf(_T("Login\n"));

						TCHAR username[TAM_COMAND], password[TAM_COMAND];

						token = _tcstok_s(NULL, _T(" "), &context);

						if (token != NULL) {
							_tcscpy_s(username, TAM_COMAND, token);
							token = _tcstok_s(NULL, _T(" "), &context);
						}

						if (token != NULL) {
							_tcscpy_s(password, TAM_COMAND, token);
						}

						_tprintf(_T("Username : %s\n"), username);
						_tprintf(_T("Password : %s\n"), password);

						WaitForSingleObject(pdata->bolsaData->hMutexData, INFINITE);

						for (DWORD i = 0; i < MAX_USERS; i++) {
							if (_tcscmp(pdata->bolsaData->users[i].userName, username) == 0) {
								if (_tcscmp(pdata->bolsaData->users[i].password, password) == 0) {

									pdata->bolsaData->users[i].ativo = TRUE;
									_tprintf(_T("Utilizador %s logado com sucesso\n"), pdata->bolsaData->users[i].userName);

									pdata->bolsaData->users[i].hPipe = pdata->hPipe;

									_tcscpy_s(resp.mensagem, TAM_COMAND, _T("Login com Sucesso"));
									resp.sucesso = TRUE;

									break;
								}
								else {
									_tprintf(_T("Password incorreta\n"));
									_tcscpy_s(resp.mensagem, TAM_COMAND, _T("Password incorreta"));
									resp.sucesso = FALSE;
								}
							}
							else if (_tcscmp(pdata->bolsaData->users[i].userName, username) != 0 && i == MAX_USERS - 1){
								_tprintf(_T("Utilizador não encontrado\n"));
								_tcscpy_s(resp.mensagem, TAM_COMAND, _T("Utilizador nao encontrado"));
								resp.sucesso = FALSE;
							}
						}

						ReleaseMutex(pdata->bolsaData->hMutexData);
					}
				
					else if (_tcscmp(pdata->resp.mensagem, _T("exit")) == 0) {
						//logout
						_tprintf(_T("exit\n"));

						WaitForSingleObject(pdata->bolsaData->hMutexData, INFINITE);

						_tcscpy_s(resp.mensagem, TAM_COMAND, _T("Logout com sucesso, BYE"));
						pdata->continua = FALSE;


						for (int i = 0; i < MAX_USERS; i++)
							if (pdata->bolsaData->users[i].hPipe == pdata->hPipe)
								pdata->bolsaData->users[i].ativo = FALSE;


						ReleaseMutex(pdata->bolsaData->hMutexData);

					}
					
					//Done probably...
					else if (_tcscmp(pdata->resp.mensagem, _T("balance")) == 0) {
						//saldo

						WaitForSingleObject(pdata->bolsaData->trinco, INFINITE);

						for (DWORD i = 0; i < MAX_USERS; i++) {

							if (pdata->hPipe == pdata->bolsaData->users[i].hPipe) {
								resp.sucesso = TRUE;
								_stprintf_s(resp.mensagem, TAM, _T("Saldo: %.2f"), pdata->bolsaData->users[i].saldo);
								break;
							}
						
						}

						ReleaseMutex(pdata->bolsaData->trinco);

					}
					
					else if (_tcscmp(pdata->resp.mensagem, _T("buy")) == 0) {
						//compra
						DWORD auxNumAcoes = 0;

						WaitForSingleObject(pdata->bolsaData->hMutexData, INFINITE);

						if (!pdata->bolsaData->stop){

							for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
								if (_tcscmp(pdata->bolsaData->company[i].name, _T("")) != 0)
									_tprintf(_T("Empresa %d: %s - Valor por ação : %.2f - Número de ações : %d\n"), i + 1, pdata->bolsaData->company[i].name, pdata->bolsaData->company[i].valor, pdata->bolsaData->company[i].numAcoes[0]);
							}

							//verificar se a empresa existe
							for (int i = 0; i < MAX_EMPRESAS; i++) {
								if (_tcscmp(pdata->bolsaData->company[i].name, pdata->resp.operacao.nomeEmpresa) == 0) {

									//ver quantas acoes tem a empresa
									for (int j = 0; j < MAX_USERS; j++)
										if (pdata->bolsaData->company[i].numAcoes[j] >= 0) {
											_tprintf(_T("AuxNumAcoes -> %d\n"), auxNumAcoes);
											_tprintf(_T("NumAcoes -> %d\n"), pdata->bolsaData->company[i].numAcoes[j]);
											auxNumAcoes += pdata->bolsaData->company[i].numAcoes[j];
										}

									_tprintf(_T("A empresa %s tem %d acoes\n"), pdata->bolsaData->company[i].name, auxNumAcoes);

									if (auxNumAcoes >= pdata->resp.operacao.quantidadeAcoes) {

										for (int j = 0; j < MAX_USERS; j++) {
											if (pdata->bolsaData->users[j].hPipe == pdata->hPipe) { // ver qual o user que esta a fazer a compra
												if (pdata->bolsaData->users[j].saldo >= pdata->resp.operacao.quantidadeAcoes * pdata->bolsaData->company[i].valor) { // ver se tem saldo suficiente

													DWORD quantidadeAComprar = pdata->resp.operacao.quantidadeAcoes;


													WaitForSingleObject(pdata->bolsaData->hMutex, INFINITE);

													pdata->bolsaData->memory->isCompra = TRUE;
													pdata->bolsaData->memory->venda.numAcoes = quantidadeAComprar;
													pdata->bolsaData->memory->venda.valor = pdata->bolsaData->company[i].valor * quantidadeAComprar;
													_tcscpy_s(pdata->bolsaData->memory->venda.name, TAM, pdata->bolsaData->users[j].userName);

													ReleaseMutex(pdata->bolsaData->hMutex);

													for(int k = 0; k < MAX_USERS && quantidadeAComprar > 0; k++) {
														if (pdata->bolsaData->company[i].numAcoes[k] > 0) {
															DWORD acoesDisponiveis = pdata->bolsaData->company[i].numAcoes[k];


															if (acoesDisponiveis >= quantidadeAComprar) {

																//se o user que esta a vender for o mesmo que esta a comprar, entao nao fazer nada
																if (_tcscmp(pdata->bolsaData->users[j].userName, pdata->bolsaData->company[i].usersVenda[k].userName) == 0)
																	continue;

																pdata->bolsaData->company[i].numAcoes[k] -= quantidadeAComprar;

																//Por o saldo na conta do user que esta a vender
																if ( pdata->bolsaData->company[i].usersVenda[k].userName != NULL || _tcscmp(pdata->bolsaData->company[i].usersVenda[k].userName, _T("")) != 0) {
																
																	for (int l = 0; l < MAX_ACOES_USER; l++) {
																		if (_tcscmp(pdata->bolsaData->users[l].userName, pdata->bolsaData->company[i].usersVenda[k].userName) == 0) {
																			pdata->bolsaData->users[l].saldo += pdata->bolsaData->company[i].valor * quantidadeAComprar;
																			break;
																		}

																	}

																}

																//Por na wallet do user
																for (int l = 0; l < MAX_ACOES_USER; l++){

																	//se ja tiver a acao, entao so acrescentar
																	if (_tcscmp(wallet.acoes[l].name, pdata->bolsaData->company[i].name) == 0) {
																		_tprintf(_T("Acao ja existe na wallet vou acrescentar \n"));
																		wallet.acoes[l].numAcoes += quantidadeAComprar;
																		break;
																	}
																	//se nao tiver a acao, entao por na wallet
																	else if (wallet.acoes[l].name == NULL || _tcscmp(wallet.acoes[l].name, _T("")) == 0) {
																		_tprintf(_T("Acao nao existe na wallet, vou por uma nova\n"));											
																		_tcscpy_s(wallet.acoes[l].name, TAM ,pdata->bolsaData->company[i].name);
																		wallet.acoes[l].numAcoes = quantidadeAComprar;
																		break;
																	}

																}
																//Mostrar a wallet DEBUG
																for (int l = 0; l < MAX_ACOES_USER; l++){
																	_tprintf(_T("Wallet:"));
																	_tprintf(_T("%d Acao : %s\n"),l ,wallet.acoes[l].name);
																}

																quantidadeAComprar = 0;
															}
															else {
																pdata->bolsaData->company[i].numAcoes[k] = 0;

																//se o user que esta a vender for o mesmo que esta a comprar, entao nao fazer nada
																if (_tcscmp(pdata->bolsaData->users[j].userName, pdata->bolsaData->company[i].usersVenda[k].userName) == 0) {
																	continue;
																}

																pdata->bolsaData->company[i].numAcoes[k] -= acoesDisponiveis;

																//Por o saldo na conta do user que esta a vender
																if (pdata->bolsaData->company[i].usersVenda[k].userName != NULL || _tcscmp(pdata->bolsaData->company[i].usersVenda[k].userName, _T("")) != 0) {

																	for (int l = 0; l < MAX_ACOES_USER; l++) {

																		if (_tcscmp(pdata->bolsaData->users[l].userName, pdata->bolsaData->company[i].usersVenda[k].userName) == 0) {
																			pdata->bolsaData->users[l].saldo += pdata->bolsaData->company[i].valor * acoesDisponiveis;
																			break;
																		}

																	}

																}

																//Por na wallet do user
																for (int l = 0; l < MAX_ACOES_USER; l++) {

																	//se ja tiver a acao, entao so acrescentar
																	if (_tcscmp(wallet.acoes[l].name, pdata->bolsaData->company[i].name) == 0) {
																		wallet.acoes[l].numAcoes += acoesDisponiveis;
																	}
																	//se nao tiver a acao, entao por na wallet
																	else if (wallet.acoes[l].name == NULL || _tcscmp(wallet.acoes[l].name, _T("")) == 0) {
																		_tcscpy_s(wallet.acoes[l].name, TAM, pdata->bolsaData->company[i].name);
																		wallet.acoes[l].numAcoes = acoesDisponiveis;
																	}

																}

																quantidadeAComprar -= acoesDisponiveis;
															}


														}
													}

													//retirar saldo na conta do user que esta a comprar
													pdata->bolsaData->users[j].saldo -= pdata->resp.operacao.quantidadeAcoes * pdata->bolsaData->company[i].valor;

													_tcscpy_s(resp.mensagem, TAM, _T("Compra efetuada com sucesso"));
													resp.sucesso = TRUE;

													//por as coisas na memoria partilhada

													WaitForSingleObject(pdata->bolsaData->hMutex, INFINITE);
													CopyMemory(pdata->bolsaData->memory->topAcoes, &pdata->bolsaData->company, MAX_EMPRESAS * sizeof(CompanyShares));
													ReleaseMutex(pdata->bolsaData->hMutex);

													break;

												}
												else {
													_tcscpy_s(resp.mensagem, TAM, _T("Saldo insuficiente"));
													resp.sucesso = FALSE;
												}
											}
										}
									}
									else {
										_tcscpy_s(resp.mensagem, TAM, _T("Ações insuficientes, so ah %d"), auxNumAcoes);
										resp.sucesso = FALSE;
									}
									break;
								}

							}

							SetEvent(pdata->bolsaData->hEvent);
							ResetEvent(pdata->bolsaData->hEvent);

						}
						else{
							_tcscpy_s(resp.mensagem, TAM, _T("Função indisponivel por momentos"));
							resp.sucesso = FALSE;

						}

						ReleaseMutex(pdata->bolsaData->hMutexData);

					}
					
					else if (_tcscmp(pdata->resp.mensagem, _T("sell")) == 0) {
						//venda

						TCHAR username[TAM];

						WaitForSingleObject(pdata->bolsaData->hMutexData, INFINITE);

						if (!pdata->bolsaData->stop){

							//buscar o username do user que esta a vender
							for (int i = 0; i < MAX_USERS; i++) {
								if (pdata->bolsaData->users[i].hPipe == pdata->hPipe) {
									_tcscpy_s(username, TAM, pdata->bolsaData->users[i].userName);
									break;
								}
							}

							//verificar se o user tem a acao e se tem a quantidade de acoes que quer vender na wallet

							for (int l = 0; l < MAX_ACOES_USER; l++) {

								//se ja tiver a acao, entao so acrescentar
								if (_tcscmp(pdata->resp.operacao.nomeEmpresa, wallet.acoes[l].name) == 0) {
									//se tiver a quantidade de acoes que quer vender
									if (wallet.acoes[l].numAcoes >= pdata->resp.operacao.quantidadeAcoes) {

										wallet.acoes[l].numAcoes -= pdata->resp.operacao.quantidadeAcoes;

										for (int i = 0; i < MAX_EMPRESAS; i++) {

											if (_tcscmp(pdata->bolsaData->company[i].name, pdata->resp.operacao.nomeEmpresa) == 0) {

												for (int j = 0; j < MAX_USERS; j++) {

													//se o user ja tiver a acao, entao so acrescentar
													if (pdata->bolsaData->company[i].numAcoes[j] != NULL || pdata->bolsaData->company[i].numAcoes[j] != 0) {

														//se o user que esta a vender ja tiver algumas ações desta empresa a venda, entao so acrescentar
														if (_tcscmp(pdata->bolsaData->company[i].usersVenda[j].userName, username) == 0) {
															pdata->bolsaData->company[i].numAcoes[j] += pdata->resp.operacao.quantidadeAcoes;
															break;
														}
													}
													//se o user que esta a vender ainda nao tiver nenhuma ação desta empresa a venda, entao adicionar
													else if (pdata->bolsaData->company[i].numAcoes[j] == NULL || pdata->bolsaData->company[i].numAcoes[j] == 0) {
														pdata->bolsaData->company[i].numAcoes[j] = pdata->resp.operacao.quantidadeAcoes;
														_tcscpy_s(pdata->bolsaData->company[i].usersVenda[j].userName, TAM, username);
														break;
													}

												}

												WaitForSingleObject(pdata->bolsaData->hMutex, INFINITE);

												pdata->bolsaData->memory->isCompra = FALSE;
												pdata->bolsaData->memory->venda.numAcoes = pdata->resp.operacao.quantidadeAcoes;
												pdata->bolsaData->memory->venda.valor = pdata->bolsaData->company[i].valor;

												ReleaseMutex(pdata->bolsaData->hMutex);

												_tcscpy_s(resp.mensagem, TAM, _T("Ações postas a Venda."));
												resp.sucesso = TRUE;

												break;
											}

										}

										break;
									}
									else {
										_stprintf_s(resp.mensagem, TAM_COMAND, _T("Nao Tem ações suficientes so têm: %d"), wallet.acoes[l].numAcoes);
										resp.sucesso = FALSE;
										break;
									}

								}
								//se nao tiver a acao, entao nao fazer nada
								else if (wallet.acoes[l].name == NULL || _tcscmp(wallet.acoes[l].name, _T("")) == 0) {
									_tcscpy_s(resp.mensagem, TAM, _T("Nao tem ações para essa empresa"));
									resp.sucesso = FALSE;
									break;
								}

							}

							SetEvent(pdata->bolsaData->hEvent);
							ResetEvent(pdata->bolsaData->hEvent);

						}
						else {
							_tcscpy_s(resp.mensagem, TAM, _T("Função indisponivel por momentos"));
							resp.sucesso = FALSE;

						}

						ReleaseMutex(pdata->bolsaData->hMutexData);

					}
					
					else if (_tcscmp(pdata->resp.mensagem, _T("listc")) == 0) {
						//listc

						_tcscpy_s(resp.mensagem, TAM_COMAND, _T("listc"));
						resp.sucesso = TRUE;

						WaitForSingleObject(pdata->bolsaData->hMutexData, INFINITE);

						for (int i = 0; i < MAX_EMPRESAS; i++) {
							if (_tcscmp(pdata->bolsaData->company[i].name, _T("")) != 0) {

								_tcscpy_s(resp.listCompany[i].name, TAM, pdata->bolsaData->company[i].name);
								resp.listCompany[i].valor = pdata->bolsaData->company[i].valor;

								for (int j = 0; j < MAX_USERS; j++)
									resp.listCompany[i].numAcoes += pdata->bolsaData->company[i].numAcoes[j];

							}
								
						}

						ReleaseMutex(pdata->bolsaData->hMutexData);
					}
					
					else {
						_tprintf(_T("Comando inválido\n"));

						_tcscpy_s(resp.mensagem, TAM_COMAND, _T("Comando invalido"));
						resp.sucesso = FALSE;

					}
				}
				else {
					_tprintf(_T("Cliente %d com mensagem vazia\n"), pdata->id);
					_tcscpy_s(resp.mensagem, TAM_COMAND, _T("Comando vazio"));
					resp.sucesso = FALSE;
				}
			}
			else {
				_tprintf(_T("[ERRO] Ler do pipe do cliente %d\n"), pdata->id);
				break;
			}
		}

		//Depois mandar de volta a estrutura Response com a resposta

		if (!WriteFile(pdata->hPipe, &resp, sizeof(Response), &nBytes, NULL)) {
			_tprintf(_T("[ERRO] Escrever no pipe! (WriteFile)\n"));
			exit(-1);
		}

		_tprintf(_T("[SERVIDOR] Enviei %d bytes ao cliente %d...(WriteFile)\n"), nBytes, pdata->id);


	} while (pdata->continua);

	CloseHandle(hEvent);
	CloseHandle(pdata->hPipe);

	WaitForSingleObject(pdata->bolsaData->trinco, INFINITE);
	CloseHandle(pdata->bolsaData->hPipes[pdata->id]);
	pdata->bolsaData->hPipes[pdata->id] = NULL;
	ReleaseMutex(pdata->bolsaData->trinco);
	
	_tprintf(_T("Cliente %d saiu\n"), pdata->id);


	return 0;

}


//DONE
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

		hPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, PIPE_UNLIMITED_INSTANCES, sizeof(Response), sizeof(Response), 1000, NULL);
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
				dataPipe[i].bolsaData = pdata;
				dataPipe[i].continua = TRUE;

				_tprintf(_T("ID -> %d\n"), dataPipe[i].id);

				_tprintf(_T("Pipe -> %d\n"), pdata->hPipes[i]);

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

	memset(&dataThreads, 0, sizeof(BolsaThreads));

	if (argc != 2) {
		_tprintf(_T("Usage: %s <file_name.txt>\n"), argv[0]);
		return 1;
	}

	//Buscar os users todos ao ficheiro.
	if (lerFicheiroUsers(argv[1], &dataThreads.users))
		_tprintf(_T("Dados lidos com sucesso do ficheiro Users\n"));

	if (!InicializaAll(&dataThreads))
		exit(1);

	//Criar a thread que vai tratar da criação dos Named Pipes e das threads para cada cliente.
	hThread[0] = CreateThread(NULL, 0, createPipe, (LPVOID)&dataThreads, 0, NULL);
	if (hThread[0] == NULL) {
		_tprintf(_T("[ERRO] Criar a thread! (CreateThread)\n"));
		exit(-1);
	}



	do {

		_tprintf(_T("\n > "));
		_fgetts(comand, TAM_COMAND, stdin);
		comand[_tcslen(comand) - 1] = '\0'; //Retirar o \n
		leComand(comand, &dataThreads);

	} while (_tcscmp(comand, _T("close")) != 0);
	

    //avisar todos os clientes que o servidor está fechando e fechar as coisas que existem abertas

	return 0;
}