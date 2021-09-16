#include <unistd.h>
#include <sys/stat.h>
#include "defines.h"

#define qwer 3

int main(int argc, char *argv[])
{
    static config_t config;
    static all_structs_t all_s;
    static all_types_t all_t;
    static read_hfiles_t hs;
    FILE *cfg;
    int a[((((qwer))))];
    a[1] = 5;
    a[1]++;

    printf("\n=======================Начало=============================\n");
    // установка стандартных настроек
    set_default(&config, &all_s);

    // разбор параметров запуска
    if (!parse_options(argc, argv, &config))
        exit(1);

    // считывание данных из конфиг файла
    if (!(cfg = fopen(config.file_paths.cfg, "rt")))
        error_open_file(config.file_paths.cfg);
    read_config_file(&config, cfg);

    // добавление стандартных типов
    default_types_in_struct(&config, &all_s);

    // чтение *.h файла
    memset(&hs, 0, sizeof(read_hfiles_t));
    read_h_file(&all_s, &hs, config.file_paths.def, config.file_name);

    // отделение индексов от имени переменной и печать справочных данных по структурам
    format_structs(&config, &all_s);

    get_size_and_sm(&config, &all_s);

    write_bdtypes(&config, &all_s, &all_t); // запись файла BdTypes

    if (config.debug == 1)
    {
        print_all_structs("./info_struct.txt", &all_s);
        print_all_defines("./info_defines.txt", &all_s);
        print_all_fields("./info_fields.txt", &all_s);
        print_buf("./info_buf.txt", &all_s);
    }

    read_config_and_write_files(&config, &all_s, &all_t, cfg); // считывание структур из конфигурационного файла и заполнение новых файлов на основе прочитанных данных

    fclose(cfg);

    printf("\n=======================Конец==============================\n\n");

    return 0;
}
