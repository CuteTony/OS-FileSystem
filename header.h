#ifndef OS_FILESYSTEM_HEADER_H
#define OS_FILESYSTEM_HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <time.h>

/**
 * �궨��
 */
#define  TRUE               1           //TRUE ����
#define  CMD_LENGTH         30          //�����л�������С
#define  BLOCK_SIZE         1024        //���̿��С
#define  SIZE               1024000     //������̿ռ��С
#define  END                65535       //FAT�е��ļ�������־
#define  FREE               0           //FAT���̿���б�־
#define  ROOT_BLOCK_NUM     2           //��Ŀ¼����ռ�̿�����
#define  MAX_OPEN_FILE      10          //���ͬʱ���ļ�����

/**
 * ˽��ʱ�����Ͷ���
 */
typedef struct TIME{
    int hour;
    int minute;
    int second;
} my_time;

/**
 * ˽���������Ͷ���
 */
typedef struct DATE{
    int year;
    int month;
    int day;
} my_date;

/**
 * ���� FAT16 ���õ��ļ����ƿ�ṹ
 */
typedef struct FCB {
    char filename[8];                   //�ļ���
    char extname[3];                    //�ļ����� FAT16 ���õ�
    unsigned char attribute;            //�ļ������ֶ�,Ϊ�����������ֻΪ�ļ��������������ԣ�ֵΪ 0 ʱ��ʾĿ¼�ļ���ֵΪ 1 ʱ��ʾ�����ļ�
    my_time time;                       //�ļ�����ʱ��
    my_date date;                       //�ļ���������
    unsigned short first;               //�ļ���ʼ�̿��
    unsigned long length;               //�ļ����ȣ��ֽ�����
    char free;                          //��ʾĿ¼���Ƿ�Ϊ�գ���ֵΪ 0����ʾ�գ�ֵΪ 1����ʾ�ѷ���
} fcb;

/**
 * �ļ������ṹ
 */
typedef struct FAT {
    unsigned short id;
} fat;

/**
 * �û����ļ���ṹ
 */
typedef struct USER_OPEN {
    char filename[8];                   //�ļ���
    char extname[3];                    //�ļ���չ��
    unsigned char attribute;            //�ļ����ԣ�ֵΪ 0 ʱ��ʾĿ¼�ļ���ֵΪ 1 ʱ��ʾ�����ļ�
    my_time time;                       //�ļ�����ʱ��
    my_date date;                       //�ļ���������
    unsigned short first;               //�ļ���ʼ�̿��
    unsigned long length;               //�ļ����ȣ��������ļ����ֽ�������Ŀ¼�ļ�������Ŀ¼�������
    char free;                          //��ʾĿ¼���Ƿ�Ϊ�գ���ֵΪ 0����ʾ�գ�ֵΪ 1����ʾ�ѷ���
    int dir_no;                         //��Ӧ���ļ���Ŀ¼���ڸ�Ŀ¼�ļ��е��̿��
    int dir_off;                        //��Ӧ���ļ���Ŀ¼���ڸ�Ŀ¼�ļ��� dir_no �̿��е�Ŀ¼�����
    char dir[MAX_OPEN_FILE][80];        //��Ӧ���ļ����ڵ�Ŀ¼��������������ټ���ָ���ļ��Ƿ��Ѿ���
    int count;                          //��дָ�����ļ��е�λ��
    char fcb_state;                     //�Ƿ��޸����ļ��� FCB �����ݣ�����޸�����Ϊ 1������Ϊ 0
    char t_openfile;                    //��ʾ���û��򿪱����Ƿ�Ϊ�գ���ֵΪ0����ʾΪ�գ������ʾ�ѱ�ĳ���ļ�ռ��
} user_open;

/**
 * ������ṹ
 */
typedef struct BLOCK0 {
    char magic_number[8];               //�ļ�ϵͳ��ħ��
    char information[200];              //�洢һЩ������Ϣ������̿��С�����̿������������ļ�����
    unsigned short root;                //��Ŀ¼�ļ�����ʼ�̿��
    unsigned char *start_block;         //�����������������ʼλ��
} block0;

/**
 * ȫ�ֱ�������
 */
FILE *fptr;                             //�ļ�ָ��
char general_buffer[SIZE];              //ͨ�û�����
unsigned char *my_v_hard;               //ָ��������̵���ʼ��ַ
user_open open_file_list[MAX_OPEN_FILE];//�û����ļ�������
user_open *ptr_current_dir;             //ָ���û����ļ����еĵ�ǰĿ¼���ڴ��ļ������λ��
char current_dir[80];                   //��¼��ǰĿ¼��Ŀ¼��������Ŀ¼��·����
unsigned char *start_position;          //��¼�����������������ʼλ��

/**
 * ��������
 */
void start_sys();

void my_format();

void my_cd(char *dirname);

void my_mkdir(char *dirname);

void my_rmdir(char *dirname);

void my_ls(void);

int my_create(char *filename);

void my_rm(char *filename);

int my_open(char *filename);

void my_close(int fd);

int my_write(int fd);

int do_write(int fd, char *text, int len, char w_style);

int my_read(int fd, int len);

int do_read(int fd, int len, char *text);

void my_exit_sys();

void init_file_system();

void init_cmd();

#endif
