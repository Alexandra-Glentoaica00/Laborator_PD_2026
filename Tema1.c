#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <wchar.h>

int main() {

    HKEY hkey = HKEY_CURRENT_USER;
    LPCWSTR SubKey = L"Control Panel\\Desktop";
    HKEY hkeyResult = NULL;
    int status = 0;

    DWORD dwIndex = 0;
    PWSTR valueName = NULL;
    DWORD dwValueNameSize = 0, maxValueNameSize = 0;
    DWORD dwType;
    PBYTE bData = NULL;
    DWORD dwDataSize = 0, maxDataSize = 0;
    
    //Deschidere subcheie
    if (RegOpenKeyExW(hkey, 
        SubKey, 
        0, 
        KEY_READ, 
        &hkeyResult) != ERROR_SUCCESS) {

        fwprintf(stderr, L"Eroare! Subcheia nu a putut fi deschisa!\n");
        status = 1;
        goto cleanup;
    }

    //Informatii suplimentare despre subcheia deschisa 
    if (RegQueryInfoKeyW(
        hkeyResult,
        NULL, NULL, NULL, NULL, NULL, NULL,
        NULL,
        &maxValueNameSize, // Longest value name length (in characters)
        &maxDataSize, // Longest value data length (in bytes)
        NULL, NULL) != ERROR_SUCCESS) {

        fwprintf(stderr, L"Eroare la interogarea informațiilor despre cheie!\n");
        status = 1;
        goto cleanup;
    }

    valueName = (PWSTR)malloc((maxValueNameSize + 1) * sizeof(WCHAR)); //+1 -> '\0'
    bData = (PBYTE)malloc(maxDataSize);

    if (!valueName || !bData) {
        fwprintf(stderr, L"Eroare! Memoria nu a putut fi alocata!\n");
        status = 1;
        goto cleanup;
    }

  
    while (1) {
        //Resetam aceste variabile in fiecare loop pentru ca vor fi modificate dupa apelul functiei
        dwValueNameSize = maxValueNameSize + 1;
        dwDataSize = maxDataSize;

        LONG result = RegEnumValueW(
            hkeyResult,
            dwIndex,
            valueName,
            &dwValueNameSize,
            NULL,
            &dwType,
            bData,
            &dwDataSize
        );

        if (result == ERROR_NO_MORE_ITEMS) {
            wprintf(L"\nToate valorile au fost citite!\n");
            break;
        }

        else if (result != ERROR_SUCCESS) {
            fwprintf(stderr, L"\nEroare la indexul %lu\n", dwIndex);
            status = 1;
            goto cleanup;
        }
       
        else {
            wprintf(L"[%lu] Nume: %-25s | Tip: ", dwIndex, valueName);

            switch (dwType) {

            case REG_SZ:
            case REG_EXPAND_SZ:
                wprintf(L"REG_SZ    | Valoare: %s\n", (wchar_t*)bData);
                break;
            case REG_DWORD:
                wprintf(L"REG_DWORD | Valoare: %u\n", *((DWORD*)bData));
                break;
            case REG_BINARY:
                wprintf(L"REG_BINARY| Valoare: [Date Binare, lungime %lu bytes]\n", dwDataSize);
                break;
            default:
                wprintf(L"Alt Tip   | Cod tip: %lu\n", dwType);
                break;
            }
        }

        dwIndex++;
    }

cleanup:
    // Inchidere subcheie + eliberare memorie
    if (hkeyResult) {
        RegCloseKey(hkeyResult);
        hkeyResult = NULL;
    }
    if (valueName) {
        free(valueName);
        valueName = NULL;
    }
    if (bData) {
        free(bData);
        bData = NULL;
    }

    return status;
}