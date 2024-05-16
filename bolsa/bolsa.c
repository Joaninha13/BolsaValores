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
			token = _tcstok_s(NULL, _T(" "), &nextToken);
		}

		if (token != NULL) {
			comp[i].numAcoes[0] = _tstoi(token);
			token = _tcstok_s(NULL, _T(" "), &nextToken);
		}

		if (token != NULL) {
			comp[i].valor = _tstof(token);
		}

		i++;
	}

	// Fecha o arquivo após a leitura
	fclose(arquivo);

	return TRUE;
}

BOOL escreveCli(HANDLE* hPipes, Response resp) {


	DWORD nBytes;
	OVERLAPPED ov;
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); //Evento para avisar que ja leu....

	ZeroMemory(&ov, sizeof(OVERLAPPED));

	ov.hEvent = hEvent;

	for (int i = 0; i < NCLIENTES; i++) {

		if (hPipes[i] != NULL) {

			if (!WriteFile(hPipes[i], &resp, sizeof(Response), &nBytes, &ov)) {

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

BOOL leComand(TCHAR comand[TAM_COMAND], BolsaThreads* data) {

	int auxi = 0;
	Response resp;

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
				_tprintf(_T("%s adicionada com sucesso\n"), data->company[i].name);
				break;
			}
		}

		CopyMemory(data->memory->topAcoes, &data->company, sizeof(CompanyShares));
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


		CopyMemory(data->memory->topAcoes, &data->company, sizeof(CompanyShares));

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

		for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
			WaitForSingleObject(data->hMutexData, INFINITE);

			if (_tcscmp(data->company[i].name, secon) == 0) {
				data->company[i].valor = _tstof(third);
				_tprintf(_T("O valor da ação da empresa %s foi alterado para %.2f\n"), data->company[i].name, data->company[i].valor);

				// por um valor na resposta com o nome da empresa e o valor novo

				_sntprintf_s(resp.mensagem, TAM, _T("O valor da ação da empresa %s foi alterado para %.2f\n"), data->company[i].name, data->company[i].valor);

				if (escreveCli(data->hPipes, resp)){
					_tprintf(_T("Mensagem enviada com sucesso\n"));
				}
				else {
					_tprintf(_T("Erro ao enviar mensagem\n"));
				}

				break;
			}


			CopyMemory(data->memory->topAcoes, &data->company, sizeof(CompanyShares));
			SetEvent(data->hEvent);

			ReleaseMutex(data->hMutexData);
		}
	}
	//Done
	else if (_tcscmp(first, _T("users")) == 0) {
		//Permite listar todos os utilizadores registados, mostrando o seu username, saldo atual e estado (ativo ou inativo).

		for (DWORD i = 0; i < MAX_USERS; i++) {
			WaitForSingleObject(data->hMutexData, INFINITE);
			if (_tcscmp(data->users[i].password, _T("")) != 0) {
				_tprintf(_T("Utilizador %d: %s - Saldo : %.2f - Estado : %s\n"), i + 1, data->users[i].userName, data->users[i].saldo, data->users[i].ativo ? _T("Ativo") : _T("Inativo"));
				ReleaseMutex(data->hMutexData);
			}else
				break;
		}

	}

	else if (_tcscmp(first, _T("pause")) == 0) {
		//Este comando faz com que as operações de compra e venda sejam suspensas (ignoradas) durante um período de tempo.Qualquer pedido de compra e venda que surja nesse período não será concretizado.
	}
	
	else if (_tcscmp(first, _T("close")) == 0) {
		//Permite encerrar o sistema. Todos os clientes e boards serão notificados, devendo terminar de seguida. 

		_tcscpy_s(resp.mensagem, TAM, _T("close"));

		escreveCli(data->hPipes, resp);

	}

	else {
		_tprintf(_T("Comando inválido\n"));

	}

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

						_tcscpy_s(resp.mensagem, TAM_COMAND, _T("Logout com sucesso, BYE"));
						pdata->continua = FALSE;

						WaitForSingleObject(pdata->bolsaData->hMutexData, INFINITE);

						for (int i = 0; i < MAX_USERS; i++){
							if (pdata->bolsaData->users[i].hPipe == pdata->hPipe) {
								pdata->bolsaData->users[i].ativo = FALSE;
							}
							
						}

						ReleaseMutex(pdata->bolsaData->hMutexData);

					}
					
					//Done probably...
					else if (_tcscmp(pdata->resp.mensagem, _T("balance")) == 0) {
						//saldo

						for (DWORD i = 0; i < MAX_USERS; i++) {
							WaitForSingleObject(pdata->bolsaData->trinco, INFINITE);

							if (pdata->hPipe == pdata->bolsaData->users[i].hPipe) {
								resp.sucesso = TRUE;
								_stprintf_s(resp.mensagem, TAM_COMAND, _T("Saldo: %.2f"), pdata->bolsaData->users[i].saldo);
							}

							ReleaseMutex(pdata->bolsaData->trinco);
						
						}

					}
					
					else if (_tcscmp(pdata->resp.mensagem, _T("buy")) == 0) {
						//compra
						int auxNumAcoes = 0;

						WaitForSingleObject(pdata->bolsaData->hMutexData, INFINITE);
						for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
							if (_tcscmp(pdata->bolsaData->company[i].name, _T("")) != 0)
								_tprintf(_T("Empresa %d: %s - Valor por ação : %.2f - Número de ações : %d\n"), i + 1, pdata->bolsaData->company[i].name, pdata->bolsaData->company[i].valor, pdata->bolsaData->company[i].numAcoes[0]);
						}
						ReleaseMutex(pdata->bolsaData->hMutexData);

						WaitForSingleObject(pdata->bolsaData->hMutexData, INFINITE);

						for (int i = 0; i < MAX_EMPRESAS; i++) {
							if (_tcscmp(pdata->bolsaData->company[i].name, pdata->resp.operacao.nomeEmpresa) == 0) {

								for (int j = 0; j < MAX_USERS; j++){
									if(pdata->bolsaData->company[i].numAcoes[j] != 0)
										auxNumAcoes += pdata->bolsaData->company[i].numAcoes[j];
								}


								if (auxNumAcoes >= pdata->resp.operacao.quantidadeAcoes) {
									for (int j = 0; j < MAX_USERS; j++) {
										if (pdata->bolsaData->users[j].hPipe == pdata->hPipe) { // ver qual o user que esta a fazer a compra
											if (pdata->bolsaData->users[j].saldo >= pdata->resp.operacao.quantidadeAcoes * pdata->bolsaData->company[i].valor) { // ver se tem saldo suficiente

												int quantidadeAComprar = pdata->resp.operacao.quantidadeAcoes;

												
												for(int k = 0; k < MAX_USERS && quantidadeAComprar > 0; k++) {
													if (pdata->bolsaData->company[i].numAcoes[k] > 0) {
														int acoesDisponiveis = pdata->bolsaData->company[i].numAcoes[k];


														if (acoesDisponiveis >= quantidadeAComprar) {

															//se o user que esta a vender for o mesmo que esta a comprar, entao nao fazer nada
															if (_tcscmp(pdata->bolsaData->users[j].userName, pdata->bolsaData->company[i].usersVenda[k].userName) == 0) {
																continue;
															}

															pdata->bolsaData->company[i].numAcoes[k] -= quantidadeAComprar;

															//Por o saldo na conta do user que esta a vender
															if ( pdata->bolsaData->company[i].usersVenda[k].userName != NULL || _tcscmp(pdata->bolsaData->company[i].usersVenda[k].userName, _T("")) != 0) {
																
																for (int l = 0; l < MAX_ACOES_USER; l++) {
																
																	if (_tcscmp(pdata->bolsaData->users[l].userName, pdata->bolsaData->company[i].usersVenda[k].userName) == 0) {
																		pdata->bolsaData->users[l].saldo += pdata->bolsaData->company[i].valor * quantidadeAComprar;
																	}

																}

															}

															pdata->bolsaData->memory->isCompra = TRUE;
															pdata->bolsaData->memory->venda.numAcoes = quantidadeAComprar;
															pdata->bolsaData->memory->venda.valor = pdata->bolsaData->company[i].valor;
															SetEvent(pdata->bolsaData->hEvent);

															//Por na wallet do user
															for (int l = 0; l < MAX_ACOES_USER; l++){

																//se ja tiver a acao, entao so acrescentar
																if (_tcscmp(wallet.acoes[l].name, pdata->bolsaData->company[j].name) == 0) {
																	wallet.acoes[l].numAcoes += quantidadeAComprar;
																}
																//se nao tiver a acao, entao por na wallet
																else if (wallet.acoes[l].name == NULL || _tcscmp(wallet.acoes[l].name, _T("")) == 0) {
																	_tcscpy_s(wallet.acoes[l].name, TAM ,pdata->bolsaData->company[j].name);
																	wallet.acoes[l].numAcoes = quantidadeAComprar;
																}

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
																	}

																}

																pdata->bolsaData->memory->isCompra = TRUE;
																pdata->bolsaData->memory->venda.numAcoes = acoesDisponiveis;
																pdata->bolsaData->memory->venda.valor = pdata->bolsaData->company[i].valor;
																SetEvent(pdata->bolsaData->hEvent);
															}

															//Por na wallet do user
															for (int l = 0; l < MAX_ACOES_USER; l++) {

																//se ja tiver a acao, entao so acrescentar
																if (_tcscmp(wallet.acoes[l].name, pdata->bolsaData->company[j].name) == 0) {
																	wallet.acoes[l].numAcoes += acoesDisponiveis;
																}
																//se nao tiver a acao, entao por na wallet
																else if (wallet.acoes[l].name == NULL || _tcscmp(wallet.acoes[l].name, _T("")) == 0) {
																	_tcscpy_s(wallet.acoes[l].name, TAM, pdata->bolsaData->company[j].name);
																	wallet.acoes[l].numAcoes = acoesDisponiveis;
																}

															}

															quantidadeAComprar -= acoesDisponiveis;
														}


													}
												}

												pdata->bolsaData->users[j].saldo -= pdata->resp.operacao.quantidadeAcoes * pdata->bolsaData->company[i].valor;

												_tcscpy_s(resp.mensagem, TAM, _T("Compra efetuada com sucesso"));
												resp.sucesso = TRUE;

												//por as coisas na memoria partilhada

												CopyMemory(pdata->bolsaData->memory->topAcoes, &pdata->bolsaData->company, sizeof(CompanyShares));
												SetEvent(pdata->bolsaData->hEvent);

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
							}

						}

						ReleaseMutex(pdata->bolsaData->hMutexData);

					}
					
					else if (_tcscmp(pdata->resp.mensagem, _T("sell")) == 0) {
						//venda

						TCHAR username[TAM];

						WaitForSingleObject(pdata->bolsaData->hMutexData, INFINITE);
						
						//buscar o username do user que esta a vender
						for (int i = 0; i < MAX_USERS; i++) {
							if (pdata->bolsaData->users[i].hPipe == pdata->hPipe) {
								_tcscpy_s(username, TAM, pdata->bolsaData->users[i].userName);
								break;
							}
						}

						//verificar se o user tem a acao e se tem a quantidade de acoes que quer vender na wallet
					
						for (int l = 0; l < MAX_ACOES_USER; l++){

							//se ja tiver a acao, entao so acrescentar
							if (_tcscmp(pdata->resp.operacao.nomeEmpresa, wallet.acoes[l].name) == 0) {
								//se tiver a quantidade de acoes que quer vender
								if (wallet.acoes[l].numAcoes >= pdata->resp.operacao.quantidadeAcoes) {

									wallet.acoes[l].numAcoes -= pdata->resp.operacao.quantidadeAcoes;

									for (int i = 0; i < MAX_EMPRESAS; i++) {

										if (_tcscmp(pdata->bolsaData->company[i].name, pdata->resp.operacao.nomeEmpresa) == 0) {

											for (int j = 0; j < MAX_USERS; j++) {

												
												if (pdata->bolsaData->company[i].numAcoes[j] != NULL || pdata->bolsaData->company[i].numAcoes[j] != 0) {

													if (_tcscmp(pdata->bolsaData->company[i].usersVenda[j].userName, username) == 0) {
														pdata->bolsaData->company[i].numAcoes[j] += pdata->resp.operacao.quantidadeAcoes;
														break;
													}
												}

												else if (pdata->bolsaData->company[i].numAcoes[j] == NULL || pdata->bolsaData->company[i].numAcoes[j] == 0) {
													pdata->bolsaData->company[i].numAcoes[j] = pdata->resp.operacao.quantidadeAcoes;
													_tcscpy_s(pdata->bolsaData->company[i].usersVenda[j].userName, TAM, username);
													break;
												}

											}

											pdata->bolsaData->memory->isCompra = FALSE;
											pdata->bolsaData->memory->venda.numAcoes = pdata->resp.operacao.quantidadeAcoes;
											pdata->bolsaData->memory->venda.valor = pdata->bolsaData->company[i].valor;
											SetEvent(pdata->bolsaData->hEvent);
											ResetEvent(pdata->bolsaData->hEvent);

											_tcscpy_s(resp.mensagem, TAM, _T("Venda efetuada com sucesso"));
											resp.sucesso = TRUE;

											break;
										}

									}

								}
								else {
									_tcscpy_s(resp.mensagem, TAM, _T("Acoes insuficientes"));
									resp.sucesso = FALSE;
								}

							}
							//se nao tiver a acao, entao nao fazer nada
							else if (wallet.acoes[l].name == NULL || _tcscmp(wallet.acoes[l].name, _T("")) == 0) {
								_tcscpy_s(resp.mensagem, TAM, _T("Acao nao encontrada"));
								resp.sucesso = FALSE;
							}

						}

						ReleaseMutex(pdata->bolsaData->hMutexData);

					}
					
					else if (_tcscmp(pdata->resp.mensagem, _T("listc")) == 0) {
						//listc

						resp.sucesso = TRUE;

						WaitForSingleObject(pdata->bolsaData->hMutexData, INFINITE);

						for (int i = 0; i < MAX_EMPRESAS; i++) {
							if (_tcscmp(pdata->bolsaData->company[i].name, _T("")) != 0) {


								_tcscpy_s(resp.listCompany[i].name, TAM, pdata->bolsaData->company[i].name);
								resp.listCompany[i].valor = pdata->bolsaData->company[i].valor;

								for (int j = 0; j < MAX_USERS; j++){
									resp.listCompany[i].numAcoes += pdata->bolsaData->company[i].numAcoes[j];
								}
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