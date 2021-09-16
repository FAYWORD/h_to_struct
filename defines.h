#ifndef DEFINES_H_
#define DEFINES_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#if !defined(_WIN32) && !defined(_WIN64)
#define TMP_PATH "/tmp/"
#else
#define TMP_PATH "./"
#endif

#define CFG_FILE "config.cfg"
#define TYPES_FILE "types.txt"
#define STRUCT_FILE "struct.txt"
#define STRUCT_INFO "_struct.info.h"
#define BASE_PATH "./cfg/"

#define MAX_STR_SIZE 2048     // максимальная длина строки
#define MAX_LEN_FILE_PATH 512 // максимальная длина пути к файлу
#define PARAM_FILE_EXT ".par" // расширение файла, куда записываются структуры и их смещение
#define STD_SYMBOL '!'        // стандартный символ начала строки для анализа в конфигурационном файле

#define FN_CREATE_BD "CreateBD"
#define FN_DROP_BD "DropBD"

#define MAX_STRUCT_COUNT 1024  // максимальное количество структур
#define MAX_FIELD_COUNT 65536  //	MAX_STRUCT_COUNT*64		// максимальное число полей в структуре
#define MAX_INDEX_COUNT 262144 //	MAX_FIELD_COUNT*4		// максимальная размерость массива переменной
#define MAX_TXT_BUF 104857600  //	100*1024*1024(100мб)	// максимальная размерость текстового буфера
#define MAX_DEFINE_COUNT 2048  // максимальное количество define
#define MAX_BD_TYPES 4096      // максимальное количество считанных типов из bdTypes

#define MAX_DEPTH 32 // максимальная вложенность include

#include "func.h"
#include "write.h"
#include "parse_hfile.h"

int parse_options(int argc, char *argv[], config_t *config);
int format_structs(config_t *c, all_structs_t *all_s);
int get_size_and_sm(config_t *c, all_structs_t *all_s);

#endif /* DEFINES_H_ */
