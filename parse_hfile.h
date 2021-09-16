/*
 * parse_hfile.h
 *
 *  Created on: 22 дек. 2017 г.
 *      Author: Иван
 */

#ifndef PARSE_HFILE_H_
#define PARSE_HFILE_H_

enum statements
{
    ST_NONE = 0,       // Не выражение
    ST_PREPROC,        // Выражение препроцессора (#)
    ST_DEFINE_NAME,    // имя считываемого define
    ST_DEFINE_VALUE,   // значение считываемого define
    ST_INCLUDE_VALUE,  // значение считываемого include
    ST_TYPEDEF_STRUCT, // Выражение typedef struct
    ST_TS_OPEN,        // ожидания символа {
    ST_FIELD,          // считывание полей структуры
    ST_TS_CLOSE        // ожидания имени структуры
};

enum comments
{
    COMMENT_NONE = 0, // Не коментарий
    COMMENT_START,    // предположенеи на начало коментария
    COMMENT_1,        // коментарий вида /* */
    COMMENT_1_END,    // коментарий вида /* */  (предположение на конец коментария)
    COMMENT_2         // комментарий вида //
};

enum fields
{
    F_TYPE = 0, // тип переменной
    F_NAME,     // имя переменной
    F_INDEX     // индексы ( если они есть)
};

typedef struct
{
    enum statements s; // состояния
    enum comments c;   // комментарии
    enum fields f;     // если обрабатываются поля структуры
    char ch;           // текущий считываемый символ
    char word[MAX_STR_SIZE];
    int wi; // индекс записи символа в слово
    int in_word;
    char file_path[MAX_LEN_FILE_PATH];
    char file_name[MAX_LEN_FILE_PATH];
    size_t max_len_word; // максимальная длина считываемого слова (или слов в случае коментариев)
    int comment_s;       // признак для сохранения коментария к структуре
    int pz;              // признак запятой
} read_hfile_t;

typedef struct
{
    read_hfile_t h[MAX_DEPTH];
    int depth;
} read_hfiles_t;

int write_in_word(read_hfile_t *h);
int delete_with_word(read_hfile_t *h);
int free_word(read_hfile_t *h);
int process_word(all_structs_t *all_s, read_hfiles_t *hs, char *file_dir);
int read_h_file(all_structs_t *all_s, read_hfiles_t *hs, char *file_dir, char *file_name);
int calc_sizes(all_structs_t *all_s, config_t *c);
int calc_indexes(all_structs_t *all_s);
int read_file_types(config_t *c, all_structs_t *all_s);

#endif /* PARSE_HFILE_H_ */
