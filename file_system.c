#include "header.h"

/**
 * myfsys �ļ�ϵͳʵ���ļ�·����
 */
char *SYS_FILE = "myfsys";

/**
 * �ļ�ϵͳ��ʼ��
 */
void start_sys() {
    unsigned char buffer[SIZE];
    unsigned char flag[8] = {'1', '0', '1', '0', '1', '0', '1', '0'};
    my_v_hard = (unsigned char *) malloc(SIZE);

    if (fopen_s(&fptr, SYS_FILE, "r") == 0) {
        fread(buffer, SIZE, 1, fptr);
        if (memcmp(flag, buffer, sizeof(flag)) == 0) {
            memcpy(my_v_hard, buffer, SIZE);
        } else {
            init_file_system();
        }
    } else {
        init_file_system();
    }

    fwrite(my_v_hard, SIZE, 1, fptr);
    fclose(fptr);

    fcb *root = (fcb *) (my_v_hard + BLOCK_SIZE * 5);
    user_open *current = &open_file_list[0];
    strcpy_s(current->filename, sizeof(root->filename), root->filename);
    strcpy_s(current->extname, sizeof(root->extname), root->extname);
    current->attribute = root->attribute;
    current->time = root->time;
    current->date = root->date;
    current->first = root->first;
    current->length = root->length;
    current->free = root->free;
    current->dir_no = 5;
    current->dir_off = 0;
    strcpy_s(current->dir[0], sizeof("/"), "/");
    current->count = 0;
    current->fcb_state = 0;
    current->t_openfile = 1;

    ptr_current_dir = current;
    start_position = ((block0 *) my_v_hard)->start_block;
    strcpy_s(current_dir, sizeof("/"), "/");
}

/**
 * ���̸�ʽ��
 */
void my_format() {
    int i;
    block0 *boot = (block0 *) my_v_hard;
    time_t now;
    struct tm now_time;
    time(&now);
    localtime_s(&now_time, &now);
    char info[] = "189050712 ����� �� FAT �ļ�ϵͳ";

    strcpy_s(boot->magic_number, sizeof("10101010"), "10101010");
    strcpy_s(boot->information, sizeof(info), info);
    boot->root = 5;
    boot->start_block = my_v_hard + BLOCK_SIZE * 5;

    fat *fat1 = (fat *) (my_v_hard + BLOCK_SIZE);
    fat *fat2 = (fat *) (my_v_hard + BLOCK_SIZE * 3);

    for (i = 0; i < 5; i++) {
        fat1[i].id = END;
        fat2[i].id = END;
    }
    fat1[5].id = 6;
    fat1[6].id = END;
    fat2[5].id = 6;
    fat2[6].id = END;
    for (i += ROOT_BLOCK_NUM; i < (SIZE / BLOCK_SIZE); i++) {
        fat1[i].id = FREE;
        fat2[i].id = FREE;
    }

    fcb *ptr_current_dir_fcb = (fcb *) (my_v_hard + BLOCK_SIZE * 5);
    strcpy_s(ptr_current_dir_fcb->filename, sizeof("."), ".");
    strcpy_s(ptr_current_dir_fcb->extname, sizeof(""), "");
    ptr_current_dir_fcb->attribute = 0;
    ptr_current_dir_fcb->time.hour = now_time.tm_hour;
    ptr_current_dir_fcb->time.minute = now_time.tm_min;
    ptr_current_dir_fcb->time.second = now_time.tm_sec;
    ptr_current_dir_fcb->date.year = now_time.tm_year;
    ptr_current_dir_fcb->date.month = now_time.tm_mon;
    ptr_current_dir_fcb->date.day = now_time.tm_mday;
    ptr_current_dir_fcb->first = 5;
    ptr_current_dir_fcb->length = 2 * sizeof(fcb);
    ptr_current_dir_fcb->free = 1;

    fcb *ptr_father_dir_fcb = ptr_current_dir_fcb + 1;
    memcpy(ptr_father_dir_fcb, ptr_current_dir_fcb, sizeof(fcb));
    strcpy_s(ptr_father_dir_fcb->filename, sizeof(".."), "..");

    for (i = ROOT_BLOCK_NUM; i < (int) (BLOCK_SIZE / sizeof(fcb)); i++) {
        ptr_father_dir_fcb++;
        strcpy_s(ptr_father_dir_fcb->filename, sizeof(""), "");
        ptr_father_dir_fcb->free = 0;
    }
}

/**
 * ���ĵ�ǰĿ¼
 *
 * @param dirname   �µĵ�ǰĿ¼��Ŀ¼��
 */
void my_cd(char *dirname) {
    if (ptr_current_dir->attribute == 1) {
        puts("Error(my_cd): ��ǰ�����ļ��У���ʹ�� 'close' ����ر��ļ�");
        return;
    }

    int i, fd = -1, tag = -1;
    static char buffer[SIZE];
    ptr_current_dir->count = 0;
    do_read(ptr_current_dir - open_file_list, (int) ptr_current_dir->length, buffer);

    fcb *fcb_ptr = (fcb *) buffer;
    for (i = 0; i < (int) (ptr_current_dir->length / sizeof(fcb)); i++, fcb_ptr++) {
        if (strcmp(fcb_ptr->filename, dirname) == 0 && fcb_ptr->attribute == 0) {
            tag = 1;
            break;
        }
    }
    if (tag != 1) {
        puts("Error(my_cd): ����Ŀ��·��������");
        return;
    } else {
        if (strcmp(fcb_ptr->filename, ".") == 0) {
            return;
        } else if (strcmp(fcb_ptr->filename, "..") == 0) {
            if (ptr_current_dir - open_file_list == 0) {
                return;
            } else {
                my_close(ptr_current_dir - open_file_list);
                return;
            }
        } else {
            for (int j = 0; j < MAX_OPEN_FILE; j++) {
                if (open_file_list[j].t_openfile == 0) {
                    open_file_list[j].t_openfile = 1;
                    fd = j;
                    break;
                }
            }
            if (fd == -1) {
                puts("Error(my_cd): �ѳ�����������ͬʱ�򿪸���");
                return;
            }
            open_file_list[fd].attribute = fcb_ptr->attribute;
            open_file_list[fd].count = 0;
            open_file_list[fd].date = fcb_ptr->date;
            open_file_list[fd].time = fcb_ptr->time;
            strcpy_s(open_file_list[fd].filename, sizeof(fcb_ptr->filename) + 1, fcb_ptr->filename);
            strcpy_s(open_file_list[fd].extname, sizeof(fcb_ptr->extname), fcb_ptr->extname);
            open_file_list[fd].first = fcb_ptr->first;
            open_file_list[fd].free = fcb_ptr->free;
            open_file_list[fd].fcb_state = 0;
            open_file_list[fd].length = fcb_ptr->length;
            strcat(strcat(strcpy((char *) open_file_list[fd].dir, (char *) (ptr_current_dir->dir)), dirname), "/");
            open_file_list[fd].t_openfile = 1;
            open_file_list[fd].dir_no = ptr_current_dir->first;
            open_file_list[fd].dir_off = i;
            ptr_current_dir = open_file_list + fd;
        }
    }

}

/**
 * ������Ŀ¼
 *
 * @param dirname   �½�Ŀ¼��Ŀ¼��
 */
void my_mkdir(char *dirname) {
    char *separator = ".", *tok_buffer, *filename = strtok_s(dirname, separator, &tok_buffer);
    static char buffer[SIZE];

    char *extname = strtok_s(NULL, separator, &tok_buffer);
    if (extname != NULL) {
        puts("Error(mkdir): ��������Ŀ¼�����ʹ����չ��");
        return;
    }

    ptr_current_dir->count = 0;
    int file_length = do_read(ptr_current_dir - open_file_list, (int) ptr_current_dir->length, buffer);
    fcb *fcb_ptr = (fcb *) buffer;

    for (int i = 0; i < (int) (file_length / sizeof(fcb)); i++) {
        if (strcmp(dirname, fcb_ptr[i].filename) == 0 && fcb_ptr->attribute == 0) {
            puts("Error(mkdir): ·�����Ѵ���");
            return;
        }
    }

    int fd = -1;
    for (int i = 0; i < MAX_OPEN_FILE; i++) {
        if (open_file_list[i].t_openfile != 1) {
            open_file_list[i].t_openfile = 1;
            fd = i;
            break;
        }
    }
    if (fd == -1) {
        puts("Error(mkdir): �ѳ�����������ͬʱ�򿪸���");
        return;
    }

    unsigned short int block_num = END;
    fat *fat_ptr = (fat *) (my_v_hard + BLOCK_SIZE);
    for (int i = 0; i < (int) (SIZE / BLOCK_SIZE); i++) {
        if (fat_ptr[i].id == FREE) {
            block_num = i;
            break;
        }
    }
    if (block_num == END) {
        puts("Error(mkdir): ���̿�����");
        return;
    }

    // ���� FAT ��
    fat *fat1 = (fat *) (my_v_hard + BLOCK_SIZE), *fat2 = (fat *) (my_v_hard + BLOCK_SIZE * 3);
    fat1[block_num].id = END;
    fat2[block_num].id = END;

    int no;
    for (no = 0; no < (int) (file_length / sizeof(fcb)); no++) {
        if (fcb_ptr[no].free == 0) {
            break;
        }
    }

    ptr_current_dir->count = no * (int) sizeof(fcb);
    ptr_current_dir->fcb_state = 1;

    // ��ʼ�� FCB
    time_t now;
    struct tm now_time;
    time(&now);
    localtime_s(&now_time, &now);

    fcb *new_fcb = (fcb *) malloc(sizeof(fcb));
    new_fcb->attribute = 0;
    new_fcb->time.hour = now_time.tm_hour;
    new_fcb->time.minute = now_time.tm_min;
    new_fcb->time.second = now_time.tm_sec;
    new_fcb->date.year = now_time.tm_year;
    new_fcb->date.month = now_time.tm_mon;
    new_fcb->date.day = now_time.tm_mday;
    strcpy_s(new_fcb->filename, strlen(dirname) + 1, dirname);
    strcpy_s(new_fcb->extname, sizeof(""), "");
    new_fcb->first = block_num;
    new_fcb->length = 2 * sizeof(fcb);
    new_fcb->free = 1;
    do_write(ptr_current_dir - open_file_list, (char *) new_fcb, sizeof(fcb), 2);

    // ���ô��ļ�����
    open_file_list[fd].attribute = 0;
    open_file_list[fd].count = 0;
    open_file_list[fd].date = new_fcb->date;
    open_file_list[fd].time = new_fcb->time;
    open_file_list[fd].dir_no = ptr_current_dir->first;
    open_file_list[fd].dir_off = no;
    strcpy_s(open_file_list[fd].filename, strlen(dirname) + 1, dirname);
    strcpy_s(open_file_list[fd].extname, sizeof(""), "");
    open_file_list[fd].fcb_state = 0;
    open_file_list[fd].first = new_fcb->first;
    open_file_list[fd].free = new_fcb->free;
    open_file_list[fd].length = new_fcb->length;
    open_file_list[fd].t_openfile = 1;
    strcat(strcat(strcpy(open_file_list[fd].dir, (char *) (ptr_current_dir->dir)), dirname), "/");


    // ����Ŀ¼��
    fcb *ptr_current_dir_fcb = (fcb *) malloc(sizeof(fcb));
    ptr_current_dir_fcb->attribute = 0;
    ptr_current_dir_fcb->date = new_fcb->date;
    ptr_current_dir_fcb->time = new_fcb->time;
    strcpy_s(ptr_current_dir_fcb->filename, sizeof("."), ".");
    strcpy_s(ptr_current_dir_fcb->extname, sizeof(""), "");
    ptr_current_dir_fcb->first = block_num;
    ptr_current_dir_fcb->length = 2 * sizeof(fcb);
    ptr_current_dir_fcb->free = 1;
    do_write(fd, (char *) ptr_current_dir_fcb, sizeof(fcb), 2);

    fcb *ptr_father_dir_fcb = (fcb *) malloc(sizeof(fcb));
    memcpy(ptr_father_dir_fcb, ptr_current_dir_fcb, sizeof(fcb));
    strcpy_s(ptr_father_dir_fcb->filename, sizeof(".."), "..");
    ptr_father_dir_fcb->first = ptr_current_dir->first;
    ptr_father_dir_fcb->length = ptr_current_dir->length;
    ptr_father_dir_fcb->date = ptr_current_dir->date;
    ptr_father_dir_fcb->time = ptr_current_dir->time;
    ptr_father_dir_fcb->free = 1;
    do_write(fd, (char *) ptr_father_dir_fcb, sizeof(fcb), 2);

    my_close(fd);
    free(new_fcb);
    free(ptr_current_dir_fcb);
    free(ptr_father_dir_fcb);


    // �޸ĸ�Ŀ¼ FCB
    fcb_ptr = (fcb *) buffer;
    fcb_ptr->length = ptr_current_dir->length;
    ptr_current_dir->count = 0;
    do_write(ptr_current_dir - open_file_list, (char *) fcb_ptr, sizeof(fcb), 2);
    ptr_current_dir->fcb_state = 1;
}

/**
 * ɾ����Ŀ¼
 *
 * @param dirname   ��ɾ��Ŀ¼��Ŀ¼��
 */
void my_rmdir(char *dirname) {
    int i, tag = 0;
    static char buffer[SIZE];

    if (strcmp(dirname, ".") == 0 || strcmp(dirname, "..") == 0) {
        puts("Error(My_Rmdir): �Ƿ��������޷��� '.' �� '..' Ŀ¼ִ�� 'rmdir'");
        return;
    }
    ptr_current_dir->count = 0;
    do_read(ptr_current_dir - open_file_list, (int) ptr_current_dir->length, buffer);

    // ����Ҫɾ����Ŀ¼
    fcb *fcb_ptr = (fcb *) buffer;
    for (i = 0; i < (int) (ptr_current_dir->length / sizeof(fcb)); i++, fcb_ptr++) {
        if (fcb_ptr->free == 0)
            continue;
        if (strcmp(fcb_ptr->filename, dirname) == 0 && fcb_ptr->attribute == 0) {
            tag = 1;
            break;
        }
    }
    if (tag != 1) {
        puts("Error(My_Rmdir): �Ҳ�����Ŀ¼");
        return;
    }
    // �޷�ɾ���ǿ�Ŀ¼
    if (fcb_ptr->length > 2 * sizeof(fcb)) {
        puts("Error(My_Rmdir): ��ֹ�Էǿ�Ŀ¼ִ�� 'rmdir'");
        return;
    }

    // ���� FAT ��
    int block_num = fcb_ptr->first;
    fat *fat1 = (fat *) (my_v_hard + BLOCK_SIZE);
    int nxt_num;
    while (1) {
        nxt_num = fat1[block_num].id;
        fat1[block_num].id = FREE;
        if (nxt_num != END) {
            block_num = nxt_num;
        } else {
            break;
        }
    }
    fat1 = (fat *) (my_v_hard + BLOCK_SIZE);
    fat *fat2 = (fat *) (my_v_hard + BLOCK_SIZE * 3);
    memcpy(fat2, fat1, BLOCK_SIZE * 2);

    // ���� FCB
    fcb_ptr->date.day = 0;
    fcb_ptr->date.month = 0;
    fcb_ptr->date.year = 0;
    fcb_ptr->time.second = 0;
    fcb_ptr->time.minute = 0;
    fcb_ptr->time.hour = 0;
    fcb_ptr->extname[0] = '\0';
    fcb_ptr->filename[0] = '\0';
    fcb_ptr->first = 0;
    fcb_ptr->free = 0;
    fcb_ptr->length = 0;

    ptr_current_dir->count = (int) (i * sizeof(fcb));
    do_write(ptr_current_dir - open_file_list, (char *) fcb_ptr, sizeof(fcb), 2);

    // �����޸ĸ�Ŀ¼ length�� ѭ��ɾ�� FCB
    int log_num = i;
    if ((log_num + 1) * sizeof(fcb) == ptr_current_dir->length) {
        ptr_current_dir->length -= sizeof(fcb);
        log_num--;
        fcb_ptr = (fcb *) buffer + log_num;
        while (fcb_ptr->free == 0) {
            fcb_ptr--;
            ptr_current_dir->length -= sizeof(fcb);
        }
    }

    // ���¸�Ŀ¼ FCB
    fcb_ptr = (fcb *) buffer;
    fcb_ptr->length = ptr_current_dir->length;
    ptr_current_dir->count = 0;
    do_write(ptr_current_dir - open_file_list, (char *) fcb_ptr, sizeof(fcb), 2);

    ptr_current_dir->fcb_state = 1;
}

/**
 * ��ʾĿ¼
 */
void my_ls(void) {
    if (ptr_current_dir->attribute == 1) {
        puts("Error(ls): �Ƿ����������ܶ��ļ�ִ�� ls ����");
        return;
    }
    char text[SIZE];

    puts("����    ����              ��С   ʱ��");
    ptr_current_dir->count = 0;
    do_read(ptr_current_dir - open_file_list, (int) ptr_current_dir->length, text);

    fcb *fcb_ptr = (fcb *) text;
    for (int i = 0; i < (int) (ptr_current_dir->length / sizeof(fcb)); i++, fcb_ptr++) {
        if (fcb_ptr->free == 1) {
            if (fcb_ptr->attribute == 0) {
                printf("<DIR>   %-15s  -     %d/%02d/%02d %02d:%02d:%02d\n",
                       fcb_ptr->filename,
                       fcb_ptr->date.year + 1900,
                       fcb_ptr->date.month,
                       fcb_ptr->date.day,
                       fcb_ptr->time.hour,
                       fcb_ptr->time.minute,
                       fcb_ptr->time.second);
            } else if (fcb_ptr->attribute == 1) {
                printf("<--->   %-15s  %-4lu  %d/%02d/%02d %02d:%02d:%02d\n",
                       fcb_ptr->filename,
                       fcb_ptr->length,
                       fcb_ptr->date.year + 1900,
                       fcb_ptr->date.month,
                       fcb_ptr->date.day,
                       fcb_ptr->time.hour,
                       fcb_ptr->time.minute,
                       fcb_ptr->time.second);
            } else {
                puts("Error(ls): �ļ�ϵͳ��������δ֪����");
                return;
            }
        }
    }

}

/**
 * �����ļ�
 *
 * @param filename  �½��ļ����ļ��������ܰ���·��
 * @return          �������ɹ������ظ��ļ����ļ����������ļ��򿪱��е������±꣩�����򷵻� -1
 */
int my_create(char *filename) {
    if (ptr_current_dir->attribute == 1) {
        puts("Error(My_Create): �Ƿ����������������ݿ���ִ�� 'create'");
        return -1;
    }

    static char buffer[SIZE];

    ptr_current_dir->count = 0;
    do_read(ptr_current_dir - open_file_list, (int) ptr_current_dir->length, buffer);

    int i;
    fcb *fcb_ptr = (fcb *) buffer;
    // �������
    for (i = 0; i < (int) (ptr_current_dir->length / sizeof(fcb)); i++, fcb_ptr++) {
        if (fcb_ptr->free == 0) {
            continue;
        }
        if (strcmp(fcb_ptr->filename, filename) == 0 && fcb_ptr->attribute == 1) {
            puts("Error(My_Create): �ļ��Ѿ�����");
            return -1;
        }
    }

    // ����� FCB
    fcb_ptr = (fcb *) buffer;
    for (i = 0; i < (int) (ptr_current_dir->length / sizeof(fcb)); i++, fcb_ptr++) {
        if (fcb_ptr->free == 0)
            break;
    }
    // ������̿鲢���� FAT ��
    int block_num = -1;
    fat *fat1 = (fat *) (my_v_hard + BLOCK_SIZE);
    for (int j = 0; j < (int) (SIZE / BLOCK_SIZE); j++) {
        if (fat1[j].id == FREE) {
            block_num = j;
            break;
        }
    }
    if (block_num == -1) {
        return -1;
    }
    fat *fat11 = (fat *) (my_v_hard + BLOCK_SIZE);
    fat *fat2 = (fat *) (my_v_hard + BLOCK_SIZE * 3);
    fat11[block_num].id = END;
    memcpy(fat2, fat11, BLOCK_SIZE * 2);

    // �޸� FCB ��Ϣ
    strcpy_s(fcb_ptr->filename, strlen(filename) + 1, filename);
    time_t now;
    struct tm now_time;
    time(&now);
    localtime_s(&now_time, &now);
    fcb_ptr->date.year = now_time.tm_year;
    fcb_ptr->date.month = now_time.tm_mon;
    fcb_ptr->date.day = now_time.tm_mday;
    fcb_ptr->time.hour = now_time.tm_hour;
    fcb_ptr->time.minute = now_time.tm_min;
    fcb_ptr->time.second = now_time.tm_sec;
    fcb_ptr->first = block_num;
    fcb_ptr->free = 1;
    fcb_ptr->attribute = 1;
    fcb_ptr->length = 0;

    ptr_current_dir->count = (int) (i * sizeof(fcb));
    do_write(ptr_current_dir - open_file_list, (char *) fcb_ptr, sizeof(fcb), 2);

    // �޸ĸ�Ŀ¼ FCB
    fcb_ptr = (fcb *) buffer;
    fcb_ptr->length = ptr_current_dir->length;
    ptr_current_dir->count = 0;
    do_write(ptr_current_dir - open_file_list, (char *) fcb_ptr, sizeof(fcb), 2);

    ptr_current_dir->fcb_state = 1;
}

/**
 * ɾ���ļ�
 *
 * @param filename ��ɾ���ļ����ļ��������ܻ�����·��
 */
void my_rm(char *filename) {
    static char buffer[SIZE];
    ptr_current_dir->count = 0;
    do_read(ptr_current_dir - open_file_list, (int) ptr_current_dir->length, buffer);

    int i, flag = 0;
    fcb *fcb_ptr = (fcb *) buffer;

    for (i = 0; i < ((int) ptr_current_dir->length / sizeof(fcb)); i++, fcb_ptr++) {
        if (strcmp(fcb_ptr->filename, filename) == 0 && fcb_ptr->attribute == 1) {
            flag = 1;
            break;
        }
    }
    if (flag != 1) {
        puts("Error(my_rm): �޴��ļ�");
        return;
    }

    // ���� FAT ��
    int block_num = fcb_ptr->first;
    fat *fat1 = (fat *) (my_v_hard + BLOCK_SIZE);
    int nxt_num = 0;
    while (TRUE) {
        nxt_num = fat1[block_num].id;
        fat1[block_num].id = FREE;
        if (nxt_num != END)
            block_num = nxt_num;
        else
            break;
    }
    fat1 = (fat *) (my_v_hard + BLOCK_SIZE);
    fat *fat2 = (fat *) (my_v_hard + BLOCK_SIZE * 3);
    memcpy(fat2, fat1, BLOCK_SIZE * 2);

    // ��� FCB
    fcb_ptr->date.day = 0;
    fcb_ptr->date.month = 0;
    fcb_ptr->date.year = 0;
    fcb_ptr->time.second = 0;
    fcb_ptr->time.minute = 0;
    fcb_ptr->time.hour = 0;
    fcb_ptr->extname[0] = '\0';
    fcb_ptr->filename[0] = '\0';
    fcb_ptr->first = 0;
    fcb_ptr->free = 0;
    fcb_ptr->length = 0;
    ptr_current_dir->count = (int) (i * sizeof(fcb));
    do_write(ptr_current_dir - open_file_list, (char *) fcb_ptr, sizeof(fcb), 2);
    //
    int log_num = i;
    if ((log_num + 1) * sizeof(fcb) == ptr_current_dir->length) {
        ptr_current_dir->length -= sizeof(fcb);
        log_num--;
        fcb_ptr = (fcb *) buffer + log_num;
        while (fcb_ptr->free == 0) {
            fcb_ptr--;
            ptr_current_dir->length -= sizeof(fcb);
        }
    }

    // �޸ĸ�Ŀ¼ . Ŀ¼�ļ��� FCB
    fcb_ptr = (fcb *) buffer;
    fcb_ptr->length = ptr_current_dir->length;
    ptr_current_dir->count = 0;
    do_write(ptr_current_dir - open_file_list, (char *) fcb_ptr, sizeof(fcb), 2);

    ptr_current_dir->fcb_state = 1;

}

/**
 * ���ļ�
 *
 * @param filename  �����ļ����ļ���
 * @return          ���򿪳ɹ������ظ��ļ��������������û����ļ����б�����ţ������򷵻� -1
 */
int my_open(char *filename) {
    static char buffer[SIZE];
    ptr_current_dir->count = 0;
    do_read(ptr_current_dir - open_file_list, (int) ptr_current_dir->length, buffer);

    int i, flag = 0;
    fcb *fcb_ptr = (fcb *) buffer;

    for (i = 0; i < (int) (ptr_current_dir->length / sizeof(fcb)); i++, fcb_ptr++) {
        if (strcmp(fcb_ptr->filename, filename) == 0 && fcb_ptr->attribute == 1) {
            flag = 1;
            break;
        }
    }
    if (flag != 1) {
        puts("Error(My_Open): �ļ�������");
        return -1;
    }

    int fd = -1;
    for (int j = 0; j < MAX_OPEN_FILE; j++) {
        if (open_file_list[j].t_openfile == 0) {
            open_file_list[j].t_openfile = 1;
            fd = j;
            break;
        }
    }
    if (fd == -1) {
        puts("Error(My_Open): �ѳ�����������ͬʱ�򿪸���");
        return -1;
    }

    open_file_list[fd].attribute = 1;
    open_file_list[fd].count = 0;
    open_file_list[fd].date = fcb_ptr->date;
    open_file_list[fd].time = fcb_ptr->time;
    open_file_list[fd].length = fcb_ptr->length;
    open_file_list[fd].first = fcb_ptr->first;
    open_file_list[fd].free = 1;
    strcpy_s(open_file_list[fd].filename, strlen(fcb_ptr->filename) + 1, fcb_ptr->filename);
    strcat(strcpy((char *) open_file_list[fd].dir, (char *) (ptr_current_dir->dir)), filename);
    open_file_list[fd].dir_no = ptr_current_dir->first;
    open_file_list[fd].dir_off = i;
    open_file_list[fd].t_openfile = 1;

    open_file_list[fd].fcb_state = 0;

    ptr_current_dir = open_file_list + fd;
    return fd;
}

/**
 * �ر��ļ�
 *
 * @param fd �ļ�������
 */
void my_close(int fd) {
    if (fd > MAX_OPEN_FILE || fd < 0) {
        puts("Error(My_Close): �Ƿ��������ļ�������Խ��");
        return;
    }

    int i, father_fd = -1;
    fcb *fcb_ptr;

    for (i = 0; i < MAX_OPEN_FILE; i++) {
        if (open_file_list[i].first == open_file_list[fd].dir_no) {
            father_fd = i;
            break;
        }
    }
    if (father_fd == -1) {
        puts("Error(My_Close): �ļ�ϵͳ���󣬸�Ŀ¼ȱʧ");
        return;
    }

    static char buffer[SIZE];
    if (open_file_list[fd].fcb_state == 1) {
        do_read(father_fd, (int) open_file_list[father_fd].length, buffer);

        fcb_ptr = (fcb *) (buffer + sizeof(fcb) * open_file_list[fd].dir_off);
        strcpy_s(fcb_ptr->extname, sizeof(open_file_list[fd].extname), open_file_list[fd].extname);
        strcpy_s(fcb_ptr->filename, sizeof(open_file_list[fd].filename), open_file_list[fd].filename);
        fcb_ptr->first = open_file_list[fd].first;
        fcb_ptr->free = open_file_list[fd].free;
        fcb_ptr->length = open_file_list[fd].length;
        fcb_ptr->time = open_file_list[fd].time;
        fcb_ptr->date = open_file_list[fd].date;
        fcb_ptr->attribute = open_file_list[fd].attribute;
        open_file_list[father_fd].count = open_file_list[fd].dir_off * (int) sizeof(fcb);

        do_write(father_fd, (char *) fcb_ptr, sizeof(fcb), 2);
    }

    // �ͷŴ��ļ���
    memset(&open_file_list[fd], 0, sizeof(user_open));
    ptr_current_dir = open_file_list + father_fd;
}

/**
 * д�ļ�
 *
 * @param fd        open() �����ķ���ֵ���ļ���������
 * @return          ʵ��д����ֽ���
 */
int my_write(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILE) {
        puts("Error(My_Write): �ļ�������");
        return -1;
    }

    int w_style;
    while (1) {
        puts("��ѡ��д��ʽ��");
        puts("1. �ض�д: ���ȫ�����ݣ���ͷ��ʼ");
        puts("2. ����д: ���ļ�ָ�봦��ʼ����д��");
        puts("3. ׷��д: ׷��д��");
        scanf_s("%d", &w_style);
        if (w_style > 3 || w_style < 1) {
            printf("input error\n");
        } else {
            break;
        }
    }
    general_buffer[0] = '\0';
    char text_tmp[SIZE] = "\0";
    puts("Info(My_Write): �����룬���к����� ':wq!' ��������");
    getchar();

    while (TRUE) {
        fgets(text_tmp, SIZE, stdin);
        if (strcmp(text_tmp, ":wq!\n") == 0) {
            break;
        }
        strcat_s(general_buffer, sizeof(general_buffer), text_tmp);
    }

    general_buffer[strlen(general_buffer)] = '\0';
    do_write(fd, general_buffer, (int) strlen(general_buffer) + 1, (char) w_style);
    open_file_list[fd].fcb_state = 1;

    return (int) strlen(general_buffer) + 1;
}

/**
 * д�ļ�����
 *
 * @param fd        open() �����ķ���ֵ���ļ���������
 * @param text      ָ��Ҫд������ݵ�ָ��
 * @param len       ����Ҫ��д���ֽ���
 * @param w_style   д��ʽ: 1-�ض�д 2-����д 3-׷��д
 * @return          ʵ��д����ֽ���
 */
int do_write(int fd, char *text, int len, char w_style) {
    unsigned char *buffer = (unsigned char *) malloc(BLOCK_SIZE);
    if (buffer == NULL) {
        puts("Error(Do_Write): �������ռ�����ʧ��");
        return -1;
    }

    int block_num = open_file_list[fd].first, read_count = 0;
    char *text_ptr = text;
    fat *fat_ptr = (fat *) (my_v_hard + BLOCK_SIZE) + block_num;

    if (w_style == 1) {
        open_file_list[fd].count = 0;
        open_file_list[fd].length = 0;
    } else if (w_style == 3) {
        open_file_list[fd].count = (int) open_file_list[fd].length;
        if (open_file_list[fd].attribute == 1) {
            // �ǿ��ļ�ɾ��ĩβ \0
            if (open_file_list[fd].length != 0) {
                open_file_list[fd].count = (int) open_file_list[fd].length - 1;
            }
        }
    }
    int offset = open_file_list[fd].count;

    // ��λ���̿�Ϳ���ƫ����
    while (offset >= BLOCK_SIZE) {
        block_num = fat_ptr->id;
        if (block_num == END) {
            puts("Error(Do_Write): ��ǰ�û����ļ���Ķ�дָ�����");
            return -1;
        }
        fat_ptr = (fat *) (my_v_hard + BLOCK_SIZE) + block_num;
        offset -= BLOCK_SIZE;
    }

    unsigned char *block_ptr = (unsigned char *) (my_v_hard + block_num * BLOCK_SIZE);
    while (len > read_count) {
        memcpy(buffer, block_ptr, BLOCK_SIZE);
        for (; offset < BLOCK_SIZE; offset++) {
            *(buffer + offset) = *text_ptr;
            text_ptr++;
            read_count++;
            if (len == read_count) {
                break;
            }
        }
        memcpy(block_ptr, buffer, BLOCK_SIZE);

        // ����һ�����̿�д�룬���û�д��̿����������
        if (offset == BLOCK_SIZE && len != read_count) {
            offset = 0;
            block_num = fat_ptr->id;
            if (block_num == END) {
                fat *new_fat = (fat *) (my_v_hard + BLOCK_SIZE);
                for (int j = 0; j < (int) (SIZE / BLOCK_SIZE); j++) {
                    if (new_fat[j].id == FREE) {
                        block_num = j;
                    }
                }
                if (block_num == END) {
                    puts("Error(Do_Write): ���̿����ʧ�ܣ��ռ�����");
                    return -1;
                }
                block_ptr = (unsigned char *) (my_v_hard + BLOCK_SIZE * block_num);
                fat_ptr->id = block_num;
                fat_ptr = (fat *) (my_v_hard + BLOCK_SIZE) + block_num;
                fat_ptr->id = END;
            } else {
                block_ptr = (unsigned char *) (my_v_hard + BLOCK_SIZE * block_num);
                fat_ptr = (fat *) (my_v_hard + BLOCK_SIZE) + block_num;
            }
        }
    }

    open_file_list[fd].count += len;
    if (open_file_list[fd].count > open_file_list[fd].length) {
        open_file_list[fd].length = open_file_list[fd].count;
    }

    // ��λ���̿�Ϳ���ƫ����
    if (w_style == 1 || (w_style == 2 && open_file_list[fd].attribute == 0)) {
        int i;
        offset = (int) open_file_list[fd].length;
        fat_ptr = (fat *) (my_v_hard + BLOCK_SIZE) + open_file_list[fd].first;

        while (offset >= BLOCK_SIZE) {
            block_num = fat_ptr->id;
            offset -= BLOCK_SIZE;
            fat_ptr = (fat *) (my_v_hard + BLOCK_SIZE) + block_num;
        }
        while (TRUE) {
            if (fat_ptr->id != END) {
                i = fat_ptr->id;
                fat_ptr->id = FREE;
                fat_ptr = (fat *) (my_v_hard + BLOCK_SIZE) + i;
            } else {
                fat_ptr->id = FREE;
                break;
            }
        }

        fat_ptr = (fat *) (my_v_hard + BLOCK_SIZE) + block_num;
        fat_ptr->id = END;
    }

    memcpy((fat *) (my_v_hard + BLOCK_SIZE * 3), (fat *) (my_v_hard + BLOCK_SIZE), BLOCK_SIZE * 2);
    return len;
}

/**
 * ���ļ�
 *
 * @param fd        open() �����ķ���ֵ���ļ���������
 * @param len       Ҫ���ļ��ж������ֽ���
 * @return          ʵ�ʶ������ֽ���
 */
int my_read(int fd, int len) {
    if (fd < 0 || fd >= MAX_OPEN_FILE) {
        puts("Error(My_Read): �ļ�������");
        return -1;
    }

    static char buffer[SIZE];
    ptr_current_dir->count = 0;
    int length = do_read(fd, len, buffer);
    puts(buffer);
    return length;
}

/**
 * ���ļ�����
 *
 * @param fd        open() �����ķ���ֵ���ļ���������
 * @param len       Ҫ����ļ��ж������ֽ���
 * @param text      ָ���Ŷ������ݵ��û�����ַ
 * @return          ʵ�ʶ������ֽ���
 */
int do_read(int fd, int len, char *text) {
    unsigned char *buffer = (unsigned char *) malloc(BLOCK_SIZE);
    if (buffer == NULL) {
        puts("Error(Do_Read): �������ռ�����ʧ��");
        return -1;
    }

    int required_len = len;
    char *text_ptr = text;
    int block_num = open_file_list[fd].first;
    int offset = open_file_list[fd].count;
    fat *fat_ptr = (fat *) (my_v_hard + BLOCK_SIZE) + block_num;

    // ��λ��ȡĿ����̿�Ϳ��ڵ�ַ
    while (offset >= BLOCK_SIZE) {
        offset -= BLOCK_SIZE;
        block_num = fat_ptr->id;
        if (block_num == END) {
            puts("Error(Do_Read): Ŀ����̿鲻����");
            return -1;
        }
        fat_ptr = (fat *) (my_v_hard + BLOCK_SIZE) + block_num;
    }

    unsigned char *block_ptr = my_v_hard + block_num * BLOCK_SIZE;
    memcpy(buffer, block_ptr, BLOCK_SIZE);

    while (len > 0) {
        if (BLOCK_SIZE - offset > len) {
            memcpy(text_ptr, buffer + offset, len);
            text_ptr += len;
            offset += len;
            open_file_list[fd].count += len;
            len = 0;
        } else {
            memcpy(text_ptr, buffer + offset, BLOCK_SIZE - offset);
            text_ptr += BLOCK_SIZE - offset;
            len -= BLOCK_SIZE - offset;

            block_num = fat_ptr->id;
            if (block_num == END) {
                break;
            }
            fat_ptr = (fat *) (my_v_hard + BLOCK_SIZE) + block_num;
            block_ptr = my_v_hard + BLOCK_SIZE * block_num;
            memcpy(buffer, block_ptr, BLOCK_SIZE);
        }
    }

    free(buffer);
    return required_len - len;
}

/**
 * �˳��ļ�ϵͳ
 */
void my_exit_sys() {
    while (ptr_current_dir - open_file_list) {
        my_close(ptr_current_dir - open_file_list);
    }

    if (fopen_s(&fptr, SYS_FILE, "w") != 0) {
        puts("Error(My_Exit): �ļ�ָ�����ʧ��");
        return;
    }
    fwrite(my_v_hard, SIZE, 1, fptr);
    fclose(fptr);
    puts("Info(My_Exit): �ѱ��沢�˳��ļ�ϵͳ");
}

/**
 * �ļ�ϵͳ��ʼ��
 */
void init_file_system() {
    if (fopen_s(&fptr, SYS_FILE, "w") != 0) {
        puts("Error(My_Format): �ļ�ָ�����ʧ��");
        return;
    }
    puts("Error(My_Format): myfsys �ļ�ϵͳ�����ڣ����ڿ�ʼ�����ļ�ϵͳ");
    my_format();
}

/**
 * �����г�ʼ��
 */
void init_cmd() {
    puts("------------------------------------�����в���ָ��------------------------------------");
    puts("mkdir��������Ŀ¼  rmdir��ɾ����Ŀ¼  ls����ʾĿ¼�е�����  cd�����ĵ�ǰĿ¼  create�������ļ�");
    puts("open�����ļ�  close���ر��ļ�  write��д�ļ�  read�����ļ�  rm��ɾ���ļ�  exit���˳��ļ�ϵͳ");
    puts("---------------------!Σ�ղ���!     format����ʽ������     !Σ�ղ���!--------------------");
    puts("------------------------------------------------------------------------------------");
}