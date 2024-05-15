
Copiar código
#include "Utils.h"

BOOL isAuthenticated = FALSE;

BOOL sendOperation(HANDLE hPipe, Response* response) {
    DWORD bytesWritten;
    OVERLAPPED ov = { 0 };
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ov.hEvent = hEvent;

    _tprintf(_T("~Entrei aqui\n"));

    BOOL success = WriteFile(hPipe, &response, sizeof(Response), &bytesWritten, &ov);
    if (!success) {
        if (GetLastError() == ERROR_IO_PENDING) {
            WaitForSingleObject(hEvent, INFINITE);

            _tprintf(_T(" Escrevi NBytes -> %d\n"), bytesWritten);

            //success = GetOverlappedResult(hPipe, &ov, &bytesWritten, FALSE);
        }
    }
    CloseHandle(hEvent);
    if (!success || bytesWritten == 0) {
        _tprintf(_T("Erro ao enviar operação: %d\n"), GetLastError());
        return FALSE;
    }
    return TRUE;
}

HANDLE connectToServer() {
    HANDLE hPipe;
    hPipe = CreateFile(
        PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
        _tprintf(_T("Erro ao conectar ao pipe: %d\n"), GetLastError());
        return NULL;
    }
    _tprintf(_T("Conectado ao servidor.\n"));
    return hPipe;
}

BOOL authenticate(HANDLE hPipe, Response* response) {
    TCHAR responseBuffer[256] = { 0 };
    DWORD bytesRead = 0;
    OVERLAPPED ov = { 0 };
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ov.hEvent = hEvent;

    if (!sendOperation(hPipe, response)) {
        _tprintf(_T("Falha ao enviar comando de login.\n"));
        CloseHandle(hEvent);
        return FALSE;
    }

    BOOL readSuccess = ReadFile(hPipe, responseBuffer, sizeof(responseBuffer), &bytesRead, &ov);
    if (!readSuccess && GetLastError() == ERROR_IO_PENDING) {
        WaitForSingleObject(hEvent, INFINITE);
        readSuccess = GetOverlappedResult(hPipe, &ov, &bytesRead, FALSE);
    }
    CloseHandle(hEvent);
    if (!readSuccess || bytesRead == 0) {
        _tprintf(_T("Falha ao receber resposta de login: %d\n"), GetLastError());
        return FALSE;
    }

    _tprintf(_T("Resposta: %s\n"), responseBuffer);
    return _tcscmp(responseBuffer, _T("Login bem-sucedido")) == 0;
}

DWORD WINAPI userInterfaceThread(LPVOID lpParam) {
    HANDLE hPipe = (HANDLE)lpParam;
    Response response = { 0 }; // Inicializa a estrutura a zeros
    TCHAR responseBuffer[1024] = { 0 };
    DWORD bytesRead = 0;

    while (TRUE) {
        if (!isAuthenticated) {
            _tprintf(_T("Digite o comando 'login <username> <password>' para autenticar ou 'exit' para encerrar: "));
        }
        else {
            _tprintf(_T("Digite um comando ('buy', 'sell', 'listc', 'balance', 'exit'): "));
        }

        _fgetts(response.operacao.msg, TAM, stdin);
        response.operacao.msg[_tcslen(response.operacao.msg) - 1] = '\0';  // Remove o caractere de nova linha

        if (_tcscmp(response.operacao.msg, _T("exit")) == 0) break;

        if (_tcsncmp(response.operacao.msg, _T("login "), 6) == 0) {
            if (authenticate(hPipe, &response)) {
                isAuthenticated = TRUE;
                _tprintf(_T("Autenticação bem-sucedida. Você agora pode executar outros comandos.\n"));
                continue;
            }
            else {
                _tprintf(_T("Falha na autenticação. Tente novamente.\n"));
                continue;
            }
        }
        else if (_tcsncmp(response.operacao.msg, _T("buy "), 4) == 0) {
            response.operacao.isCompra = TRUE;
            TCHAR* context = NULL;
            TCHAR* token = _tcstok_s(response.operacao.msg, _T(" "), &context); // saltar "buy"
            token = _tcstok_s(NULL, _T(" "), &context); // Nome da empresa
            _tcscpy_s(response.operacao.nomeEmpresa, MAX_NOME, token);
            token = _tcstok_s(NULL, _T(" "), &context); // Número de ações
            response.operacao.quantidadeAcoes = _tstoi(token);

            if (!sendOperation(hPipe, &response)) {
                _tprintf(_T("Falha ao enviar comando 'buy'.\n"));
                continue;
            }
        }
        else if (_tcsncmp(response.operacao.msg, _T("sell "), 5) == 0) {
            response.operacao.isCompra = FALSE;
            TCHAR* context = NULL;
            TCHAR* token = _tcstok_s(response.operacao.msg, _T(" "), &context); // saltar "sell"
            token = _tcstok_s(NULL, _T(" "), &context); // Nome da empresa
            _tcscpy_s(response.operacao.nomeEmpresa, MAX_NOME, token);
            token = _tcstok_s(NULL, _T(" "), &context); // Número de ações
            response.operacao.quantidadeAcoes = _tstoi(token);

            if (!sendOperation(hPipe, &response)) {
                _tprintf(_T("Falha ao enviar comando 'sell'.\n"));
                continue;
            }
        }
        else if (!isAuthenticated) {
            _tprintf(_T("Por favor, autentique-se usando o comando 'login <username> <password>'.\n"));
            continue;
        }
        else {
            if (!sendOperation(hPipe, &response)) {
                _tprintf(_T("Falha ao enviar comando: %d\n"), GetLastError());
                continue;
            }
        }

        OVERLAPPED ovRead = { 0 };
        HANDLE hEventRead = CreateEvent(NULL, TRUE, FALSE, NULL);
        ovRead.hEvent = hEventRead;

        BOOL readSuccess = ReadFile(hPipe, responseBuffer, sizeof(responseBuffer), &bytesRead, &ovRead);
        if (!readSuccess && GetLastError() == ERROR_IO_PENDING) {
            WaitForSingleObject(hEventRead, INFINITE);
            readSuccess = GetOverlappedResult(hPipe, &ovRead, &bytesRead, FALSE);
        }
        CloseHandle(hEventRead);
        if (!readSuccess || bytesRead == 0) {
            _tprintf(_T("Falha ao receber resposta: %d\n"), GetLastError());
            continue;
        }

        _tprintf(_T("Resposta: %s\n"), responseBuffer);
    }

    return 0;
}

int _tmain(int argc, TCHAR* argv[]) {
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif 

    HANDLE hPipe = connectToServer();
    if (hPipe == NULL) {
        _tprintf(_T("Não foi possível conectar ao servidor.\n"));
        return 1;
    }

    DWORD threadId;
    HANDLE hThread = CreateThread(NULL, 0, userInterfaceThread, hPipe, 0, &threadId);
    if (hThread == NULL) {
        _tprintf(_T("Falha ao criar a thread de interface do usuário: %d\n"), GetLastError());
        CloseHandle(hPipe);
        return 1;
    }

    WaitForSingleObject(hThread, INFINITE);

    CloseHandle(hThread);
    CloseHandle(hPipe);
    return 0;
}