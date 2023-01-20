#include "../Common/Includes.h"
#include "ClientSocket.h"
#include <time.h>

char* read_file(const char* src_path, size_t* file_size);
int write_file(const char* dest_path, char* file_contents, size_t file_size);

int main(int argc, char* argv[]) {
    clock_t start_time = clock();
    int mode = 0;

    #pragma region Code
    //Check if there are any arguments passed to the process, if not default values will be used
    //process [client_id] [client_port] [server_port] [mode]
    if (argc > 1) {
        if (argc != 5) {
            printf_s("Wrong number of arguments.\nCorrect format : .\\Process.exe [client_id] [client_port] [server_port] [mode!=0 = auto]\n");
            return -1;
        }

        set_process_id(strtol(argv[1], NULL, 10));
        set_client_port((u_short)(strtoul(argv[2], NULL, 10)));
        set_server_port((u_short)(strtoul(argv[3], NULL, 10)));
        mode = strtol(argv[4], NULL, 10);
    }

    //Start the sockets
    init_client_sockets();

    //Do something

    if (mode) {
        send_request(RegisterService, NULL);
        send_request(RegisterService, NULL);

    }
    else
    {
        printf_s("Manual control started...\n1 : RegisterService\n2 : SendData\n0 : quit\n\n");
        int iResult = 0;
        do {
            scanf_s("%d", &iResult);

            switch (iResult)
            {
            case 1:
                send_request(RegisterService, NULL);
                break;
            case 2:
                {}
                size_t file_size;
                char* file_contents = read_file("SQueue.c", &file_size);
                if (file_contents != NULL) {
                    if (write_file("kek\\SQueue.c", file_contents, file_size) == 0) {
                        printf("File replicated successfully\n");
                    }
                    free(file_contents);
                }
                break;
            case 0:

                break;
            default:
                printf_s("Unknown command.\n");
                break;
            }
        } while (iResult);
    }

    //Clean-up
    //getchar();
    cleanup(ALL_GOOD);
#pragma endregion

    clock_t end_time = clock();
    double execution_time = (double)((end_time - start_time) / CLOCKS_PER_SEC);
    printf("Execution time: %f seconds\n", execution_time);

    return 0;
}

char* read_file(const char* src_path, size_t* file_size) {
    FILE* src_file = fopen(src_path, "rb"); // open the file in binary mode
    if (src_file == NULL) {
        printf("Error opening file\n");
        return NULL;
    }

    // Get the size of the file
    fseek(src_file, 0, SEEK_END);
    *file_size = ftell(src_file);
    rewind(src_file);

    // Allocate memory for the contents of the file
    char* buffer = (char*)malloc(*file_size);
    if (buffer == NULL) {
        printf("Error allocating memory\n");
        fclose(src_file);
        return NULL;
    }

    // Read the contents of the file into the buffer
    size_t bytes_read = fread(buffer, sizeof(char), *file_size, src_file);
    if (bytes_read != *file_size) {
        printf("Error reading file\n");
        free(buffer);
        fclose(src_file);
        return NULL;
    }

    fclose(src_file);
    return buffer;
}

int write_file(const char* dest_path, char* file_contents, size_t file_size) {
    FILE* dest_file = fopen(dest_path, "wb"); // open the file in binary mode
    if (dest_file == NULL) {
        printf("Error opening file\n");
        return -1;
    }

    // Write the contents of the buffer to the file
    size_t bytes_written = fwrite(file_contents, sizeof(char), file_size, dest_file);
    if (bytes_written != file_size) {
        printf("Error writing file\n");
        fclose(dest_file);
        return -1;
    }

    fclose(dest_file);
    return 0;
}