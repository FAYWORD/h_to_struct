/*
 * func.h
 *
 *  Created on: 22 дек. 2017 г.
 *      Author: Иван
 */

#ifndef FUNC_H_
#define FUNC_H_

typedef struct
{
    char cfg[MAX_LEN_FILE_PATH];         // путь конфигурационного файла
    char types[MAX_LEN_FILE_PATH];       // путь к файлу с типами данных
    char structn[MAX_LEN_FILE_PATH];     // структуры которые не надо разбирать
    char struct_info[MAX_LEN_FILE_PATH]; //
    char prefix[MAX_LEN_FILE_PATH];      // префикс
    char def[MAX_LEN_FILE_PATH];         // путь к папке где лежит основной *h файл
    char par[MAX_LEN_FILE_PATH];         // путь к файлу *.par (описание смещений)
    char bd_create[MAX_LEN_FILE_PATH];   // путь для создания файла по созданию таблиц для бд
    char bd_drop[MAX_LEN_FILE_PATH];     // путь для создания файла по удалению таблиц бд
    char bd_data[MAX_LEN_FILE_PATH];     // путь для создания bdData.cfg
    char bd_types[MAX_LEN_FILE_PATH];    // путь для создания bdTypes.cfg
} file_paths_t;

typedef struct
{
    file_paths_t file_paths; // пути
    char symbol;             // стандартный символ начала строки в конфигурационном файле для ее анализа
    char debug;              // отладочная информация (0 - нет; 1 - да)
    char file_name[64];      // имя основного *h файла (без пути)
    int first_num_type;      // ид с которого начинать записывать типы
    int read_types;          // считывать или нет файл types.cfg
    int write_types;         // писать или нет в bdTypes.cfg стандартные типы
    int alignment;           // выравнивание
} config_t;

typedef struct
{
    unsigned int sm_type;       // тип переменной
    unsigned int sm_name;       // имя переменной
    unsigned int sm_type_std;   // стандартный тип
    unsigned int sm_comment;    // комментарий
    unsigned int m_index;       // номер в массиве описания индексов
    unsigned int m_index_count; // количество индексов массива
    unsigned int sm_type_idx;   // тип переменной с индексами
    unsigned int sz;            // размер
} struct_field_t;               // поля в структуре

typedef struct
{
    unsigned int sm_name;     // имя структуры
    unsigned int sm_name_std; // стандартный тип
    unsigned int sm_type_bd;  // имя структуры
    unsigned int sm_comment;  // комментарий
    unsigned int typeid;      // ид
    unsigned int size;        // размер структуры
    unsigned int first_field; // номер в массиве описания полей струтуры
    unsigned int nfields;     // число полей в структуре
    unsigned int sm_filepath; // путь к файлу из которого была считана структура
} struct_t;                   //структуры

typedef struct
{
    unsigned int sm_name; // смещение на имя define
    int value;            // значение
} define_t;               // define

typedef struct
{
    struct_t s[MAX_STRUCT_COUNT];
    struct_field_t f[MAX_FIELD_COUNT];
    unsigned int idx[MAX_INDEX_COUNT];
    define_t d[MAX_DEFINE_COUNT];
    char buf[MAX_TXT_BUF];
    unsigned int c_std_types;
    unsigned int c_structs;
    unsigned int c_fields;
    unsigned int c_idx;
    unsigned int c_define;
    unsigned int len_buf;
} all_structs_t;

typedef struct
{
    unsigned int sm_name;
    int num;
    int id;
} type_t;

typedef struct
{
    type_t t[MAX_BD_TYPES];
    int count;
} all_types_t;

void trim(char *s);
int format_str(char *str);
int set_default(config_t *config, all_structs_t *all_s);                         // установка начальных значений
int print_config_info(config_t *config);                                         // печать конфигурационных данных
int read_config_file(config_t *config, FILE *cfg);                               // считывание данных из конфиг файла
int write_in_buf(unsigned int *sm, char *str, char *buf, unsigned int *len_buf); // запись строки в буфер
int delete_from_buf(all_structs_t *all_s, unsigned int sm);                      // удаление строки из буфера
int rewrite_in_buf(all_structs_t *all_s, unsigned int *sm, char *str);           // перезапись строки из буфера

#define ADD_STD_TYPE(type, id, bd)                                                                 \
    {                                                                                              \
        write_in_buf(&all_s->s[all_s->c_structs].sm_name, #type, all_s->buf, &all_s->len_buf);     \
        write_in_buf(&all_s->s[all_s->c_structs].sm_name_std, #type, all_s->buf, &all_s->len_buf); \
        write_in_buf(&all_s->s[all_s->c_structs].sm_type_bd, bd, all_s->buf, &all_s->len_buf);     \
        all_s->s[all_s->c_structs].size = sizeof(type);                                            \
        all_s->s[all_s->c_structs].typeid = id;                                                    \
        write_in_buf(&all_s->s[all_s->c_structs].sm_comment, #type, all_s->buf, &all_s->len_buf);  \
        all_s->c_structs++;                                                                        \
        all_s->c_std_types++;                                                                      \
    }

int default_types_in_struct(config_t *c, all_structs_t *all_s);
int print_all_structs(char *file_path, all_structs_t *all_s);
int print_all_defines(char *file_path, all_structs_t *all_s);
int print_all_fields(char *file_path, all_structs_t *all_s);
int print_buf(char *file_path, all_structs_t *all_s);
void error_open_file(char *str);

#endif /* FUNC_H_ */
