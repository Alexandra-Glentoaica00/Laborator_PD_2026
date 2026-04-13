#include <windows.h>
#include <stdio.h>
#include <winioctl.h>

//Enunt: Afisati metadatele unui device conectat la PC

int main() {

    HANDLE hDevice; 
    DWORD bytesReturned;
    int status = 0;

    //Structuri pentru interogari - storage properties & disk geometry
    STORAGE_PROPERTY_QUERY query;
    DISK_GEOMETRY geometry;

    //Buffer pentru informatii obtinute
    BYTE buffer[1024];

    //Obtinem un handle pentru device-ul pe care dorim sa il interogam
    hDevice = CreateFileA(
        "\\\\.\\F:",            //Stick conectat la PC
        0,                       //pentru interogare de metadate
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    //Tratare posibile cazuri de eroare
    if (hDevice == INVALID_HANDLE_VALUE) {

        DWORD err = GetLastError();

        if (err == ERROR_FILE_NOT_FOUND) {
            fprintf(stderr, "Dispozitivul nu exista!\n");
            status = 1;
            goto cleanup;
        }

        else if(err == ERROR_PATH_NOT_FOUND){
            fprintf(stderr, "Cale invalida! Formatul numelui este incorect!\n");
            status = 1;
            goto cleanup;
        }

        else if (err == ERROR_ACCESS_DENIED) {
            fprintf(stderr, "Acces refuzat! Ruleaza aplicatia cu drepturi administrator!\n");
            status = 1;
            goto cleanup;
        }

        else{
            fprintf(stderr, "Cod de eroare: %lu\n", err);
            status = 1;
            goto cleanup;
        }
        
    }
    else {
        printf("Handle obtinut cu succes!\n");
    }

    //Prima interogare - obtinem informatii despre proprietatile device-ului

    ZeroMemory(&query, sizeof(query));
    query.PropertyId = StorageDeviceProperty;
    query.QueryType = PropertyStandardQuery;

    if (!DeviceIoControl(
        hDevice,
        IOCTL_STORAGE_QUERY_PROPERTY,
        &query,
        sizeof(query),
        buffer,             
        sizeof(buffer),
        &bytesReturned,
        NULL
    )) {

        DWORD err = GetLastError();

        if (err ==  ERROR_INSUFFICIENT_BUFFER) {
            fprintf(stderr, "Buffer-ul trimis nu este suficient de mare pentru a stoca datele!\n");
            status = 1;
            goto cleanup;
        }
        
        else {
            fprintf(stderr, "Cod de eroare: %lu\n", err);
            status = 1;
            goto cleanup;
        }
    }

    STORAGE_DEVICE_DESCRIPTOR* descriptor = (STORAGE_DEVICE_DESCRIPTOR*)buffer;

    printf("\n--- Proprietati ale device-ului ---\n");

    // Verificam daca offset-urile sunt valide (> 0) 
    if (descriptor->VendorIdOffset != 0 && descriptor->VendorIdOffset != (DWORD) - 1)
        printf("Vendor:   %s\n", (char*)(buffer + descriptor->VendorIdOffset));

    if (descriptor->ProductIdOffset != 0 && descriptor->ProductIdOffset != (DWORD) - 1)
        printf("Product:  %s\n", (char*)(buffer + descriptor->ProductIdOffset));

    if (descriptor->ProductRevisionOffset != 0 && descriptor->ProductRevisionOffset != (DWORD) - 1)
        printf("Revision: %s\n", (char*)(buffer + descriptor->ProductRevisionOffset));

    if (descriptor->SerialNumberOffset != 0 && descriptor->SerialNumberOffset != (DWORD) - 1)
        printf("Serial:   %s\n", (char*)(buffer + descriptor->SerialNumberOffset));


    //A doua interogare - obtinem informatii despre geometria discului

    if (!DeviceIoControl(
        hDevice,
        IOCTL_DISK_GET_DRIVE_GEOMETRY,
        NULL,
        0,
        &geometry,
        sizeof(geometry),
        &bytesReturned,
        NULL
    )) {

        DWORD err = GetLastError();

        fprintf(stderr, "Cod de eroare: %lu\n", err);
        status = 1;
        goto cleanup;
    }

    printf("\n--- Geometrie disc ---\n");

    printf("Cilindri:         %lld\n", geometry.Cylinders.QuadPart);
    printf("Sectoare/Pista:   %lu\n", geometry.SectorsPerTrack);
    printf("Bytes per Sector: %lu\n", geometry.BytesPerSector);

    // Calcul capacitate totală (Cilindri * Piste/Cilindru * Sectoare/Pistă * Bytes/Sector)
    unsigned long long totalBytes = (unsigned long long)geometry.Cylinders.QuadPart * geometry.TracksPerCylinder * geometry.SectorsPerTrack * geometry.BytesPerSector;
    printf("Capacitate:       %.2f GB\n", (double)totalBytes / (1024 * 1024 * 1024));

cleanup:

    if (hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(hDevice);
    }

    return status;
}

