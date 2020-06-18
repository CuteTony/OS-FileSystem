#include "header.h"

int main() {
    //ȫ�ֱ�����ʼ��
    //�ļ�ϵͳ��ʼ��
    start_sys();
    init_cmd();

    while (TRUE) {
        char input[CMD_LENGTH], *cmd, *param, *buffer, *separator = " ";

        printf("tony %s $", *ptr_current_dir->dir);
        gets_s(input, CMD_LENGTH);
        if (strcmp(input, "") == 0) {
            continue;
        }
        cmd = strtok_s(input, separator, &buffer);

        if (strcmp(cmd, "mkdir") == 0) {
            if ((param = strtok_s(NULL, separator, &buffer)) == NULL) {
                puts("Error(CMD): ��Ч����");
                continue;
            }
            my_mkdir(param);
        } else if (strcmp(cmd, "rmdir") == 0) {
            if ((param = strtok_s(NULL, separator, &buffer)) == NULL) {
                puts("Error(CMD): ��Ч����");
                continue;
            }
            my_rmdir(param);
        } else if (strcmp(cmd, "ls") == 0) {
            my_ls();
        } else if (strcmp(cmd, "cd") == 0) {
            if ((param = strtok_s(NULL, separator, &buffer)) == NULL) {
                puts("Error(CMD): ��Ч����");
                continue;
            }
            my_cd(param);
        } else if (strcmp(cmd, "create") == 0) {
            if ((param = strtok_s(NULL, separator, &buffer)) == NULL) {
                puts("Error(CMD): ��Ч����");
                continue;
            }
            my_create(param);
        } else if (strcmp(cmd, "open") == 0) {
            if ((param = strtok_s(NULL, separator, &buffer)) == NULL) {
                puts("Error(CMD): ��Ч����");
                continue;
            }
            my_open(param);
        } else if (strcmp(cmd, "close") == 0) {
            if (ptr_current_dir->attribute == 0) {
                puts("Error(CMD): �Ƿ����������ܶ�Ŀ¼ִ�� 'close' ����");
                continue;
            }
            my_close(ptr_current_dir - open_file_list);
        } else if (strcmp(cmd, "write") == 0) {
            if (ptr_current_dir->attribute == 1) {
                my_write(ptr_current_dir - open_file_list);
            }
        } else if (strcmp(cmd, "read") == 0) {
            if ((param = strtok_s(NULL, separator, &buffer)) == NULL) {
                puts("Error(CMD): ��Ч����");
                continue;
            }
            char *ptr;
            my_read(ptr_current_dir - open_file_list, (int) strtol(param, &ptr, 10));
        } else if (strcmp(cmd, "rm") == 0) {
            if ((param = strtok_s(NULL, separator, &buffer)) == NULL) {
                puts("Error(CMD): ��Ч����");
                continue;
            }
            my_rm(param);
        } else if (strcmp(cmd, "exit") == 0) {
            my_exit_sys();
            return 0;
        } else if (strcmp(cmd, "format") == 0) {
            my_format();
            puts("Info(CMD): �ļ�ϵͳ��ʼ�����");
        } else {
            printf("Error(CMD): ��Ч��� %s\n", cmd);
        }
    }
}
