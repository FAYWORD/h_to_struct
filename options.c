#include <getopt.h>

#include "defines.h"

int parse_options(int argc, char *argv[], config_t *config)
{
    int c;

    while (1)
    {
        static struct option opts[] =
            {
                {"config-path", 1, 0, 'C'},
                {"help", 0, 0, 'h'},
                {"debug", 0, 0, 'd'},
                {"special-symbol", 1, 0, 'S'},
                {"default-path", 1, 0, 'P'},
                {"alignment", 1, 0, 'A'},
                {0, 0, 0, 0}};

        c = getopt_long(argc, argv, "C:S:P:A:dha", (struct option *)&opts, 0);
        if (c == -1)
            break;

        switch (c)
        {
        case 'C':
            sprintf(config->file_paths.cfg, "%s%s", optarg, CFG_FILE);
            sprintf(config->file_paths.types, "%s%s", optarg, TYPES_FILE);
            sprintf(config->file_paths.structn, "%s%s", optarg, STRUCT_FILE);
            sprintf(config->file_paths.struct_info, "%s%s", optarg, STRUCT_INFO);
            break;
        case 'S':
            if (strlen(optarg) == 1)
                config->symbol = optarg[0];
            else
            {
                fprintf(stderr, "ОШИБКА!Неправильно задан параметр '-S %s'\n", optarg);
                return (0);
            }
            break;
        case 'P':
            sprintf(config->file_paths.prefix, optarg);
            break;
        case 'h':
            printf("Программа для создания конфигурационных файлов из заголовочного файла.\n");
            printf("  Версия:0.9 (compiled: "__DATE__
                   " "__TIME__
                   ")\n");
            printf("Аргументы:\n");
            printf("  -C или --config-path <путь>\t\tЗадать путь к папке с конфигурационными файлами\n");
            printf("  -S или --special-symbol 'символ'\tЗадать символ для считывания строк\n");
            printf("  -d или --debug\t\t\tОтобразить отладочную информацию\n");
            printf("  -P или --prefix-path\t\t\tПодставляемый путь к путям в конфиг файле\n");
            printf("  -A или --alignment\t\t\tВыравнивание 4 или 8 байт (если не задано, то без выравнивания) \n");
            return 0;
            break;
        case 'd':
            config->debug = 1;
            break;
        case 'A':
            config->alignment = atoi(optarg);
            if ((config->alignment != 4) && (config->alignment != 8))
            {
                fprintf(stderr, "Указано неверное значение для выравнивания. Допустимые значения: 0(без выравнивания), 4, 8\n");
                exit(1);
            }
            break;
        default:
            return 0;
        }
    }

    return 1;
}
