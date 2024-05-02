#include "Utils.h"

BOOL isAuthenticated = FALSE; 

HANDLE connectToServer() {
    HANDLE hPipe;
    hPipe = CreateFile(
        PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
        _tprintf(_T("Erro ao conectar ao pipe: %d\n"), GetLastError());
        return NULL;
    }
    _tprintf(_T("Conectado ao servidor.\n"));
    return hPipe;
}

void userInterface(HANDLE hPipe) {
    TCHAR command[256];
    TCHAR inputBuffer[256];
    TCHAR response[1024];  
    DWORD bytesRead, bytesWritten;
    BOOL success;

    while (TRUE) {
        if (!isAuthenticated) {
            _tprintf(_T("Digite 'login' para autenticar ou 'sair' para encerrar: "));
        }
        else {
            _tprintf(_T("Digite um comando ('buy', 'sell', 'listc', 'balance', 'exit') ou 'sair' para encerrar: "));
        }

        _fgetts(inputBuffer, 256, stdin);
        inputBuffer[_tcslen(inputBuffer) - 1] = '\0';  

        if (_tcscmp(inputBuffer, _T("sair")) == 0) break;

        if (!isAuthenticated && _tcscmp(inputBuffer, _T("login")) != 0) {
            _tprintf(_T("Por favor, autentique-se usando o comando 'login'.\n"));
            continue;
        }

        if (_tcscmp(inputBuffer, _T("login")) == 0) {
            if (authenticate(hPipe)) {
                isAuthenticated = TRUE;
                _tprintf(_T("Autenticação bem-sucedida. Você agora pode executar outros comandos.\n"));
                continue;
            }
            else {
                _tprintf(_T("Falha na autenticação. Tente novamente.\n"));
                continue;
            }
        }
        else if (!isAuthenticated) {
            continue; 
        }

        _stprintf_s(command, _T("%s"), inputBuffer);

        success = WriteFile(hPipe, command, (wcslen(command) + 1) * sizeof(TCHAR), &bytesWritten, NULL);
        if (!success || bytesWritten == 0) {
            _tprintf(_T("Falha ao enviar comando: %d\n"), GetLastError());
            continue;
        }

        success = ReadFile(hPipe, response, sizeof(response), &bytesRead, NULL);
        if (!success || bytesRead == 0) {
            _tprintf(_T("Falha ao receber resposta: %d\n"), GetLastError());
            continue;
        }

        _tprintf(_T("Resposta: %s\n"), response);
    }
}

BOOL authenticate(HANDLE hPipe) {
    TCHAR username[100], password[100], command[256], response[256];
    DWORD bytesWritten, bytesRead;

    _tprintf(_T("Por favor, faça login.\nUsername: "));
    _fgetts(username, 100, stdin);
    username[_tcslen(username) - 1] = '\0';

    _tprintf(_T("Password: "));
    _fgetts(password, 100, stdin);
    password[_tcslen(password) - 1] = '\0';

    _stprintf_s(command, _T("login %s %s"), username, password);

    if (!WriteFile(hPipe, command, (wcslen(command) + 1) * sizeof(TCHAR), &bytesWritten, NULL) || bytesWritten == 0) {
        _tprintf(_T("Falha ao enviar comando de login: %d\n"), GetLastError());
        return FALSE;
    }

    if (!ReadFile(hPipe, response, sizeof(response), &bytesRead, NULL) || bytesRead == 0) {
        _tprintf(_T("Falha ao receber resposta de login: %d\n"), GetLastError());
        return FALSE;
    }

    _tprintf(_T("Resposta: %s\n"), response);
    return _tcscmp(response, _T("Login bem-sucedido")) == 0;
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

    userInterface(hPipe);  

    CloseHandle(hPipe);  
    return 0;
}

