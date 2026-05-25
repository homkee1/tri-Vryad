#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int response = MessageBoxW(
        NULL,
        L"вы уверены, что хотите сбросить настройки игры TriVRyad?",
        L"сброс настроек TriVRyad",
        MB_YESNO | MB_ICONQUESTION
    );

    if (response == IDYES) {
        LSTATUS status = RegDeleteTreeW(HKEY_CURRENT_USER, L"Software\\TriVRyad");

        if (status == ERROR_SUCCESS) {
            MessageBoxW(
                NULL,
                L"настройки успешно сброшены до значений по умолчанию!!!!",
                L"lol",
                MB_OK | MB_ICONINFORMATION
            );
        }
        else if (status == ERROR_FILE_NOT_FOUND) {
            MessageBoxW(
                NULL,
                L"нет настроек в реестре",
                L"err",
                MB_OK | MB_ICONINFORMATION
            );
        }
        else {
            MessageBoxW(
                NULL,
                L"мб у процесса недостаточно прав доступа к реестру",
                L"err",
                MB_OK | MB_ICONERROR
            );
        }
    }
    return 0;
}