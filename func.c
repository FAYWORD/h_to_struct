/*
 * func.c
 *
 *  Created on: 22 дек. 2017 г.
 *      Author: Иван
 */
#include <ctype.h>

#include "defines.h"

void trim(char *s)
{
    int i = 0, j;
    while ((s[i] == ' ') || (s[i] == '\t'))
        i++;
    if (i > 0)
    {
        for (j = 0; j < strlen(s); j++)
            s[j] = s[j + i];
        s[j] = 0;
    }
    i = strlen(s) - 1;
    while ((s[i] == ' ') || (s[i] == '\t'))
        i--;
    if (i < (strlen(s) - 1))
        s[i + 1] = 0;
}

int format_str(char *str)
{
    int i, j;

    //замена пустых символов на пробелы и удаление подряд идущих пробелов
    for (i = 0; i < strlen(str);)
    {
        if (isspace(str[i]) && (str[i] != ' '))
            str[i] = ' ';
        if (isspace(str[i]) && isspace(str[i + 1]))
        {
            for (j = i; j < strlen(str); j++)
                str[j] = str[j + 1];
            continue;
        }
        i++;
    }
    // проверка и удаление первого пробела
    if (isspace(str[0]))
    {
        for (j = 0; j < strlen(str) - 1; j++)
            str[j] = str[j + 1];
        str[j] = 0;
    }
    // проверка и удаление последнего пробела
    if (isspace(str[strlen(str) - 1]))
        str[strlen(str) - 1] = 0;

    return 0;
}

int set_default(config_t *config, all_structs_t *all_s)
{
    sprintf(config->file_paths.cfg, "%s%s", BASE_PATH, CFG_FILE);
    sprintf(config->file_paths.types, "%s%s", BASE_PATH, TYPES_FILE);
    sprintf(config->file_paths.structn, "%s%s", BASE_PATH, STRUCT_FILE);
    sprintf(config->file_paths.struct_info, "%s%s", BASE_PATH, STRUCT_INFO);
    config->file_paths.prefix[0] = 0;
    config->symbol = STD_SYMBOL;
    config->alignment = 0;

    memset(all_s, 0, sizeof(all_structs_t));
    printf("Размер выделенной памяти для записи информации по структурам = %d\n", sizeof(all_structs_t));
    all_s->len_buf = 2;

    return 0;
}

int print_config_info(config_t *config)
{
    printf("Путь к конфигурационному файлу: %s\n", config->file_paths.cfg);
    printf("Путь к файлу с типами данных: %s\n", config->file_paths.types);
    printf("Путь к файлу с структурами которые не надо разбирать: %s\n", config->file_paths.structn);
    printf("Путь к файлу : %s\n", config->file_paths.struct_info);
    printf("Префикс к пути: %s\n", config->file_paths.prefix);
    printf("Путь к папке где лежит основной *h файл: %s\n", config->file_paths.def);
    printf("Путь к файлу *.par (описание смещений): %s\n", config->file_paths.par);
    printf("Путь к файлу по созданию таблиц для бд: %s\n", config->file_paths.bd_create);
    printf("Путь к файлу по удалению таблиц из бд: %s\n", config->file_paths.bd_drop);
    printf("Путь для создания bdData.cfg: %s\n", config->file_paths.bd_data);
    printf("Путь для создания bdTypes.cfg: %s\n", config->file_paths.bd_types);

    printf("Стандартный символ начала строки в конфигурационном файле для ее анализа: %c\n", config->symbol);
    printf("Отладочная информация (0 - нет; 1 - да): %d\n", config->debug);
    printf("Имя основного *h файла (без пути): %s\n", config->file_name);
    printf("Ид с которого начинать записывать типы: %d\n", config->first_num_type);
    printf("Cчитывать или нет файл types.cfg (0 - нет; 1 - да): %d\n", config->read_types);
    printf("Писать или нет в bdTypes.cfg стандартные типы (0 - нет; 1 - да): %d\n", config->write_types);

    return 0;
}

// считывание данных из конфиг файла
int read_config_file(config_t *config, FILE *cfg)
{
    static char str[MAX_STR_SIZE];
    static char tmp_path[MAX_LEN_FILE_PATH];

    fscanf(cfg, "%s", config->file_paths.def);                                       // путь к *.h файлу
    sprintf(config->file_paths.par, "%s%s", config->file_paths.def, PARAM_FILE_EXT); // имя файла для записи параметров
    strcpy(config->file_name, strrchr(config->file_paths.def, '/') + 1);             // отделение имени файла
    *(strrchr(config->file_paths.def, '/') + 1) = 0;                                 // отбрасывание имени файла от пути
    sprintf(config->file_paths.bd_create, "%s%s", config->file_paths.def, FN_CREATE_BD);
    sprintf(config->file_paths.bd_drop, "%s%s", config->file_paths.def, FN_DROP_BD);
    unlink(config->file_paths.bd_drop);

    fgets(str, MAX_STR_SIZE - 1, cfg);
    fscanf(cfg, "%s", tmp_path); //путь для создания bdData.cfg
    sprintf(config->file_paths.bd_data, "%s%s", config->file_paths.prefix, tmp_path);
    fgets(str, MAX_STR_SIZE - 1, cfg);
    fscanf(cfg, "%s", tmp_path); //путь для создания bdTypes.cfg
                                 //	sprintf(config->file_paths.bd_types, "%s%s", config->file_paths.prefix,  tmp_path);
    sprintf(config->file_paths.bd_types, "%s", tmp_path);
    fgets(str, MAX_STR_SIZE - 1, cfg);
    fscanf(cfg, "%d", &config->first_num_type); // ид с которого начинать записывать типы
    fgets(str, MAX_STR_SIZE - 1, cfg);
    fscanf(cfg, "%d", &config->read_types); // считывать или нет файл types.cfg
    fgets(str, MAX_STR_SIZE - 1, cfg);
    fscanf(cfg, "%d", &config->write_types); // писать или нет в bdTypes.cfg стандартные типы
    fgets(str, MAX_STR_SIZE - 1, cfg);

    if (config->debug == 1)
        print_config_info(config);

    return 0;
}

int write_in_buf(unsigned int *sm, char *str, char *buf, unsigned int *len_buf)
{
    static char f_str[MAX_STR_SIZE];
    static int len;

    sprintf(f_str, str);
    /*if (strlen(str) > 1)
		format_str(f_str);*/
    len = strlen(f_str);
    if (*len_buf + len + 1 >= MAX_TXT_BUF)
    {
        fprintf(stderr, "ОШИБКА! Переполнение текстового буфера (#define MAX_TXT_BUF)\n");
        exit(1);
    }
    *sm = *len_buf;
    strcpy(buf + *len_buf, f_str);
    *len_buf += len + 1;

    return 0;
}

int delete_from_buf(all_structs_t *all_s, unsigned int sm)
{
    int len;
    int i, ns, nf, nd;

    len = strlen(all_s->buf + sm) + 1;

    for (i = sm; i <= all_s->len_buf; i++)
        all_s->buf[i] = all_s->buf[i + len];
    all_s->len_buf -= len;

    for (ns = 0; ns < all_s->c_structs; ns++)
    {
        if (all_s->s[ns].sm_comment > sm)
            all_s->s[ns].sm_comment -= len;
        if (all_s->s[ns].sm_filepath > sm)
            all_s->s[ns].sm_filepath -= len;
        if (all_s->s[ns].sm_name > sm)
            all_s->s[ns].sm_name -= len;
        if (all_s->s[ns].sm_name_std > sm)
            all_s->s[ns].sm_name_std -= len;
        if (all_s->s[ns].sm_type_bd > sm)
            all_s->s[ns].sm_type_bd -= len;
    }

    for (nf = 0; nf < all_s->c_fields; nf++)
    {
        if (all_s->f[nf].sm_comment > sm)
            all_s->f[nf].sm_comment -= len;
        if (all_s->f[nf].sm_name > sm)
            all_s->f[nf].sm_name -= len;
        if (all_s->f[nf].sm_type > sm)
            all_s->f[nf].sm_type -= len;
        if (all_s->f[nf].sm_type_idx > sm)
            all_s->f[nf].sm_type_idx -= len;
        if (all_s->f[nf].sm_type_std > sm)
            all_s->f[nf].sm_type_std -= len;
    }

    for (nd = 0; nd < all_s->c_define; nd++)
    {
        if (all_s->d[nd].sm_name > sm)
            all_s->d[nd].sm_name -= len;
    }

    return 0;
}

// перезапись строки из буфера
int rewrite_in_buf(all_structs_t *all_s, unsigned int *sm, char *str)
{
    delete_from_buf(all_s, *sm);

    write_in_buf(sm, str, all_s->buf, &all_s->len_buf);

    return 0;
}

int read_file_types(config_t *c, all_structs_t *all_s)
{
    FILE *fp;
    static char str[MAX_STR_SIZE];
    unsigned int typeid;
    static char str_size[128];
    static char str_name[128];
    int i;
    int num;
    int p;

    num = all_s->c_structs;
    if (!(fp = fopen(c->file_paths.types, "rt")))
    {
        fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ! Отсутствует файл с переопределениями [%s]\n", c->file_paths.types);
        return -1;
    }

    do
    {
        fgets(str, MAX_STR_SIZE - 1, fp);
        if (feof(fp))
            break;
        if (strlen(str) < 2)
            continue;
        sscanf(str, "%d%s%s", &typeid, str_size, str_name);
        for (i = 0; i < strlen(str_size); i++)
            if (str_size[i] == '_')
                str_size[i] = ' ';
        p = 0;
        for (i = 0; i < num; i++)
            if (!strcmp(all_s->buf + all_s->s[i].sm_name, str_size))
            {
                // занесение типа данных
                all_s->s[all_s->c_structs].typeid = typeid;
                write_in_buf(&all_s->s[all_s->c_structs].sm_name, str_name, all_s->buf, &all_s->len_buf);
                write_in_buf(&all_s->s[all_s->c_structs].sm_comment, str_size, all_s->buf, &all_s->len_buf);
                all_s->s[all_s->c_structs].size = all_s->s[i].size;
                all_s->s[all_s->c_structs].sm_type_bd = all_s->s[i].sm_type_bd;
                all_s->s[all_s->c_structs].sm_name_std = all_s->s[i].sm_name;
                all_s->c_structs++;
                all_s->c_std_types++;
                p = 1;
                break;
            }
        if (p == 0)
            fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ! Тип [%s] не найден в стандартных типах. Пропуск\n", str_size);
    } while (!feof(fp));

    fclose(fp);

    return 0;
}

int default_types_in_struct(config_t *c, all_structs_t *all_s)
{
    // заносим стандартные типы
    ADD_STD_TYPE(unsigned char, 5, "character varying");
    ADD_STD_TYPE(unsigned short, 6, "smallint");
    ADD_STD_TYPE(unsigned int, 7, "integer");
    ADD_STD_TYPE(unsigned long long, 8, "bigint");
    ADD_STD_TYPE(char, 9, "character varying");
    ADD_STD_TYPE(short, 10, "smallint");
    ADD_STD_TYPE(int, 11, "integer");
    ADD_STD_TYPE(long long, 12, "bigint");
    ADD_STD_TYPE(float, 13, "real");
    ADD_STD_TYPE(double, 14, "double precision");

    //если надо читать types.txt
    if (c->read_types == 1)
        read_file_types(c, all_s);

    return 0;
}

int print_all_structs(char *file_path, all_structs_t *all_s)
{
    FILE *fp;
    int ns, nf, ni;
    int nf_s;
    char name[512];
    char idx[32];

    if (!(fp = fopen(file_path, "wt")))
    {
        fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ! Невозможно открыть файл для записи отладочной информации [%s]\n", file_path);
        return -1;
    }

    for (ns = 0; ns < all_s->c_structs; ns++)
    {
        fprintf(fp, "Struct_num=%-5d| name=%-30s| std_name=%-20s| type_bd=%-20s|  id=%-5d| sz=%-10d| ff=%-5d| cf=%-5d| comment=%-70s| file_name=%s\n",
                ns,
                all_s->buf + all_s->s[ns].sm_name,
                all_s->buf + all_s->s[ns].sm_name_std,
                all_s->buf + all_s->s[ns].sm_type_bd,
                all_s->s[ns].typeid,
                all_s->s[ns].size,
                all_s->s[ns].first_field,
                all_s->s[ns].nfields,
                all_s->buf + all_s->s[ns].sm_comment,
                all_s->buf + all_s->s[ns].sm_filepath);
        nf_s = 1;
        for (nf = all_s->s[ns].first_field; nf < all_s->s[ns].first_field + all_s->s[ns].nfields; nf++)
        {
            sprintf(name, all_s->buf + all_s->f[nf].sm_name);
            for (ni = all_s->f[nf].m_index; ni < all_s->f[nf].m_index + all_s->f[nf].m_index_count; ni++)
            {
                sprintf(idx, "[%d]", all_s->idx[ni]);
                strcat(name, idx);
            }

            fprintf(fp, "\tnum_field=%-5d%-5d;type=%-20s;name=%-20s;type_std=%-20s;comment=[%s];\n",
                    nf_s,
                    nf,
                    all_s->buf + all_s->f[nf].sm_type,
                    name,
                    all_s->buf + all_s->f[nf].sm_type_std,
                    all_s->buf + all_s->f[nf].sm_comment);

            nf_s++;
            /*for (ni = all_s->f[nf].m_index; ni < all_s->f[nf].m_index + all_s->f[nf].m_index_count; ni++)
			{

			}*/
        }
    }

    fclose(fp);

    return 0;
}

int print_all_defines(char *file_path, all_structs_t *all_s)
{
    FILE *fp;
    int nd;

    if (!(fp = fopen(file_path, "wt")))
    {
        fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ! Невозможно открыть файл для записи отладочной информации [%s]\n", file_path);
        return -1;
    }

    for (nd = 0; nd < all_s->c_define; nd++)
    {
        fprintf(fp, "%-5d|%-10d %-30s| %-100d|\n",
                nd,
                all_s->d[nd].sm_name,
                all_s->buf + all_s->d[nd].sm_name,
                all_s->d[nd].value);
    }

    fclose(fp);

    return 0;
}

int print_all_fields(char *file_path, all_structs_t *all_s)
{
    FILE *fp;
    int nf, ni;
    char name[512];
    char idx[32];

    if (!(fp = fopen(file_path, "wt")))
    {
        fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ! Невозможно открыть файл для записи отладочной информации[%s]\n", file_path);
        return -1;
    }

    for (nf = 0; nf < all_s->c_fields; nf++)
    {
        sprintf(name, all_s->buf + all_s->f[nf].sm_name);
        for (ni = all_s->f[nf].m_index; ni < all_s->f[nf].m_index + all_s->f[nf].m_index_count; ni++)
        {
            sprintf(idx, "[%d]", all_s->idx[ni]);
            strcat(name, idx);
        }

        fprintf(fp, "\tnum_field=%-5d|type=%-40s|name=%-40s|type_std=%-20s|comment=[%s]|\n",
                nf,
                all_s->buf + all_s->f[nf].sm_type,
                name,
                all_s->buf + all_s->f[nf].sm_type_std,
                all_s->buf + all_s->f[nf].sm_comment);
    }

    fclose(fp);

    return 0;
}

int print_buf(char *file_path, all_structs_t *all_s)
{
    FILE *fp;
    int i;

    if (!(fp = fopen(file_path, "wt")))
    {
        fprintf(stderr, "ПРЕДУПРЕЖДЕНИЕ! Невозможно открыть файл для записи отладочной информации[%s]\n", file_path);
        return -1;
    }

    for (i = 0; i < all_s->len_buf; i++)
    {
        if (all_s->buf[i] == 0)
            fprintf(fp, "%-10d|%s\n", i + 1, all_s->buf + (i + 1));
    }

    fclose(fp);

    return 0;
}

void error_open_file(char *str)
{
    fprintf(stderr, "ОШИБКА! Невозможно открыть файл [%s]\n", str);
    exit(1);
}
