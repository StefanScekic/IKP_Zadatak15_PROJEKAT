#include "../Common/Includes.h"
#include "ClientSocket.h"
#include <time.h>

int main(int argc, char* argv[]) {
    clock_t start_time = clock();
    int mode = 0;

    #pragma region Code

    //Check if there are any arguments passed to the process, if not default values will be used
    //process [client_id] [server_port] [mode]
    if (argc > 1) {
        if (argc != 4) {
            printf_s("Wrong number of arguments.\nCorrect format : .\\Process.exe [client_id] [server_port] [mode!=0 = auto]\n");
            return -1;
        }

        set_process_id(strtol(argv[1], NULL, 10));
        set_server_port((u_short)(strtoul(argv[2], NULL, 10)));
        mode = strtol(argv[3], NULL, 10);
    }

    char dir[100] = "";
    strcpy(dir, get_process_dir());
    CreateDirectoryA((LPCSTR)dir, NULL);

    //Start the sockets
    init_client_sockets();

    //Do something

    if (mode) {
        switch (mode)
        {
        case 1:
            send_request(RegisterService, NULL);
            Sleep(100);
            break;
        case 2:
            send_request(RegisterService, NULL);
            int i = 0;
            char test_file_name[100] = {0};
            for (i = 0; i < 5; i++) {
                switch (i%5)
                {
                case 0:
                    strcpy(test_file_name, "File0.txt");
                    break;
                case 1:
                    strcpy(test_file_name, "File1.txt");
                    break;
                case 2:
                    strcpy(test_file_name, "File2.txt");
                    break;
                case 3:
                    strcpy(test_file_name, "File3.txt");
                    break;
                case 4:
                    strcpy(test_file_name, "File4.txt");
                    break;
                default:
                    break;
                }

                char wr_location[100] = "";
                strcat(wr_location, get_process_dir());
                strcat(wr_location, "\\");
                strcat(wr_location, test_file_name);

                size_t file_size;
                printf("%s\n", wr_location);

                char* file_contents = read_file(wr_location, &file_size);
                if (file_contents != NULL) {
                    file temp_file;
                    temp_file.ownder_id = get_process_id();
                    memcpy(temp_file.file_contents, file_contents, strlen(file_contents));
                    temp_file.file_length = strlen(file_contents);
                    memcpy(temp_file.file_name, test_file_name, strlen(test_file_name));

                    send_request(SendData, &temp_file);
                    free(file_contents);
                }
            }
            break;
        default:
            printf("Nepoznati test mode");
            break;
        }

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
                printf_s("Name of the file to be replicated: ");
                char wr_location[100] = "";
                char file_name[MAX_FILE_NAME] = "";

                scanf_s("%s", file_name, MAX_FILE_NAME);
                file_name[strlen(file_name)] = '\0';

                strcat(wr_location, get_process_dir());
                strcat(wr_location, "\\");
                strcat(wr_location, file_name);

                size_t file_size;
                printf("%s\n", wr_location);

                char* file_contents = read_file(wr_location, &file_size);
                if (file_contents != NULL) {
                    file temp_file;
                    temp_file.ownder_id = get_process_id();
                    memcpy(temp_file.file_contents, file_contents, strlen(file_contents));
                    temp_file.file_length = strlen(file_contents);
                    memcpy(temp_file.file_name, file_name, strlen(file_name));

                    send_request(SendData, &temp_file);   
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