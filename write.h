/*
 * write.h
 *
 *  Created on: 25 дек. 2017 г.
 *      Author: Иван
 */

#ifndef WRITE_H_
#define WRITE_H_

#define MAX_COUNT_ID 40960       // максимальное количество выделяемых ИД для всех структур
#define MAX_DEPTH_STRUCT 32      // максимальная вложенность структуры
#define MAX_COUNT_ID_STRUCT 4096 // максимальное количество выделяемых ИД для 1 структуры

typedef struct
{
    int id;
    int type_bd;
    char prefix[128];
    char table_name[128];
    char struct_name[128];
} read_values_t;

typedef struct
{
    char typename[128];
    int ns;
    int nf;
    int first_id;
} depth_info_t;

typedef struct
{
    read_values_t rv;
    depth_info_t di[MAX_DEPTH_STRUCT];
    unsigned int array_id[MAX_COUNT_ID];
    unsigned int count_id;
    int cur_id;
    int depth;
    int max_depth;
} bd_data_t;

enum bdcreate
{
    START_QUERY = 0, // Начало запроса
    IN_QUERY,        // тело запроса (перечисление полей)
    END_QUERY,       // окончание запроса
    END              // окончание всех запросов (сбор итогового файла)
};

// определение максимального выравнивания в структуре
int get_alignment_s(all_structs_t *all_s, int ns, int *max_a, int config_a);

// разбор структуры по полям с подсчетом размера и заполнение файла *.par
int get_size_and_sm_struct(FILE *fp, all_structs_t *all_s, size_t *total_sz, int ns, int max_a, int depth);

int read_config_and_write_files(config_t *c, all_structs_t *all_s, all_types_t *all_t, FILE *cfg);
int write_bdtypes(config_t *c, all_structs_t *all_s, all_types_t *all_t);
int write_struct_in_bd_data(config_t *c, all_structs_t *all_s, all_types_t *all_t, bd_data_t *bd, FILE *bddata);

#endif /* WRITE_H_ */
