/*
 * parse_hfile.c
 *
 *  Created on: 22 дек. 2017 г.
 *      Author: Иван
 */

#include <ctype.h>

#include "defines.h"

int write_in_word(read_hfile_t *h)
{
    size_t len;

    if ((h->wi == 0) && (isspace(h->ch)))
        return 0;
    len = strlen(h->word);
    if (len + 1 >= h->max_len_word)
    {
        fprintf(stderr, "Превышена максимально допустимая длина для считывания строки (%d)", h->max_len_word);
        exit(1);
    }
    h->in_word = 1;
    h->word[h->wi++] = h->ch;

    return 0;
}

int delete_with_word(read_hfile_t *h)
{
    h->wi--;
    h->word[h->wi] = 0;

    return 0;
}

int free_word(read_hfile_t *h)
{
    memset(h->word, 0, h->max_len_word);
    h->in_word = 0;
    h->wi = 0;
    return 0;
}
int process_word(all_structs_t *all_s, read_hfiles_t *hs, char *file_dir)
{
    int nd;
    size_t len;
    char *word = hs->h[hs->depth].word;
    int i;

    if (hs->h[hs->depth].wi == 0)
        return 0;
    format_str(word);

    if (hs->h[hs->depth].c == COMMENT_NONE)
    {
        switch (hs->h[hs->depth].s)
        {
        case ST_NONE:
            if (!strcmp("typedef", word))
                hs->h[hs->depth].s = ST_TYPEDEF_STRUCT;
            break;
        case ST_PREPROC:
            if (!strcmp("define", word))
                hs->h[hs->depth].s = ST_DEFINE_NAME;
            else if (!strcmp("include", word))
                hs->h[hs->depth].s = ST_INCLUDE_VALUE;
            else
            {
                hs->h[hs->depth].s = ST_NONE;
                //fprintf(stderr, "В файле '%s' обнаружена необрабатываемая директива препроцессора (#%s)", hs->h[hs->depth].file_path, word);
                //exit(1);
                break;
            }
            break;
        case ST_DEFINE_NAME:
            write_in_buf(&all_s->d[all_s->c_define].sm_name, word, all_s->buf, &all_s->len_buf);
            hs->h[hs->depth].s = ST_DEFINE_VALUE;
            break;
        case ST_DEFINE_VALUE:
            if ((atoi(word) == 0) && (word[0] != '0')) // проверка на число
            {
                // строка
                for (nd = 0; nd < all_s->c_define; nd++)
                    if (!strcmp(word, all_s->buf + all_s->d[nd].sm_name))
                    {
                        // совпадение найдено
                        all_s->d[all_s->c_define].value = all_s->d[nd].value;
                        printf("[%s] [%s] [%d]\n", all_s->buf + all_s->d[all_s->c_define].sm_name, word, all_s->d[all_s->c_define].value);
                        //all_s->d[all_s->c_define].sm_name = all_s->d[nd].sm_name;
                        all_s->c_define++;
                        hs->h[hs->depth].s = ST_NONE;
                        break;
                    }
                if (hs->h[hs->depth].s != ST_NONE)
                {
                    len = strlen(all_s->buf + all_s->d[all_s->c_define].sm_name);
                    memset(all_s->buf + all_s->d[all_s->c_define].sm_name, 0, len);
                    all_s->len_buf -= len + 1;
                    hs->h[hs->depth].s = ST_NONE;
                }
                break;
            }
            // число
            all_s->d[all_s->c_define].value = atoi(word);
            all_s->c_define++;
            hs->h[hs->depth].s = ST_NONE;
            break;
        case ST_INCLUDE_VALUE:
        {
            int i;
            int pSl = 0;

            for (i = 0; i < strlen(word); i++)
            {
                if (word[i] == '/')
                {
                    pSl = 1;
                    break;
                }
            }
            if (pSl == 0)
            {
                printf("\tВ файле [%s] обнаружен #include \"[%s%s]\"\n", hs->h[hs->depth].file_path, file_dir, word);
                hs->depth++;
                read_h_file(all_s, hs, file_dir, hs->h[hs->depth - 1].word); // запуск считывания другого файла
            }
            else
            {
                static char new_fp[MAX_LEN_FILE_PATH];

                sprintf(new_fp, "%s%s", file_dir, word); // собираем полный путь
                strcpy(word, strrchr(new_fp, '/') + 1);  // отделяем имя файла
                *(strrchr(new_fp, '/') + 1) = 0;
                printf("\tВ файле [%s] обнаружен #include \"%s%s\"\n", hs->h[hs->depth].file_path, new_fp, word);
                hs->depth++;
                read_h_file(all_s, hs, new_fp, hs->h[hs->depth - 1].word); // запуск считывания другого файла
            }
            hs->h[hs->depth].s = ST_NONE;
            break;
        }
        case ST_TYPEDEF_STRUCT:
            if (!strcmp("struct", word))
            {
                all_s->s[all_s->c_structs].first_field = all_s->c_fields;
                hs->h[hs->depth].s = ST_TS_OPEN;
            }
            break;
        case ST_TS_OPEN:
            // имя структуры (обычно пусто)
            break;
        case ST_FIELD:
            switch (hs->h[hs->depth].f)
            {
            case F_TYPE:
                //printf("type=[%s]\n", word);
                write_in_buf(&all_s->f[all_s->c_fields].sm_type, word, all_s->buf, &all_s->len_buf);
                hs->h[hs->depth].f = F_NAME;
                hs->h[hs->depth].pz = 0;
                break;
            case F_NAME:
                if (!strcmp(word, "unsigned") ||
                    !strcmp(word, "long") ||
                    !strcmp(word, "int") ||
                    !strcmp(word, "char") ||
                    !strcmp(word, "short") ||
                    !strcmp(word, "float") ||
                    !strcmp(word, "double"))
                {
                    all_s->len_buf--;
                    write_in_buf(&len, " ", all_s->buf, &all_s->len_buf);
                    all_s->len_buf--;
                    write_in_buf(&len, word, all_s->buf, &all_s->len_buf);
                    //printf("type_sost=[%s]\n", word);
                }
                else
                {
                    //printf("[%s]\n", hs->h[hs->depth].word);
                    if (all_s->f[all_s->c_fields].sm_type == 0)
                    {
                        fprintf(stderr, "ОШИБКА! В файле '%s' после считывания поля структуры (%s %s) идет неизвестная последовательность: %s\n",
                                hs->h[hs->depth].file_path,
                                all_s->buf + all_s->f[all_s->c_fields - 1].sm_type,
                                all_s->buf + all_s->f[all_s->c_fields - 1].sm_name,
                                word);
                        exit(1);
                    }
                    hs->h[hs->depth].pz = 0;
                    write_in_buf(&all_s->f[all_s->c_fields].sm_name, word, all_s->buf, &all_s->len_buf);
                    all_s->s[all_s->c_structs].nfields++;
                    all_s->c_fields++;
                    //hs->h[hs->depth].f = F_TYPE;
                }
                break;
            case F_INDEX:
            {
                int p;
                // если индекс не число, то поиск в define и запись в индекс значение найденного define
                if (atoi(word) == 0)
                {
                    p = 0;
                    for (nd = 0; nd < all_s->c_define; nd++)
                        if (!strcmp(word, all_s->buf + all_s->d[nd].sm_name))
                        {
                            if (all_s->f[all_s->c_fields - 1].m_index_count == 0)
                                all_s->f[all_s->c_fields - 1].m_index = all_s->c_idx;
                            all_s->idx[all_s->c_idx++] = all_s->d[nd].value;
                            all_s->f[all_s->c_fields - 1].m_index_count++;
                            p = 1;
                            break;
                        }
                    if (p == 0)
                    {
                        fprintf(stderr, "ОШИБКА! Найден неизвестный define (%s) в переменной \"%s\"", word, all_s->buf + all_s->f[all_s->c_fields - 1].sm_name);
                        //for (nd = 0; nd < all_s->c_define; nd++) printf("%s %d\n", all_s->buf + all_s->d[nd].sm_name, all_s->d[nd].value);
                        exit(1);
                    }
                }
                else // если число, тогда записываем его
                {
                    if (all_s->f[all_s->c_fields - 1].m_index_count == 0)
                        all_s->f[all_s->c_fields - 1].m_index = all_s->c_idx;
                    all_s->idx[all_s->c_idx++] = atoi(word);
                    all_s->f[all_s->c_fields - 1].m_index_count++;
                }
                //rewrite_in_buf(all_s,  &all_s->f[all_s->c_fields-1].sm_name, word);
                break;
            }
            }
            //printf("[%s]\n", word);
            break;
        case ST_TS_CLOSE:
            // запись имени структуры
            write_in_buf(&all_s->s[all_s->c_structs].sm_name, word, all_s->buf, &all_s->len_buf);
            for (i = 0; i < all_s->c_structs; i++)
            {
                if (!strcmp(hs->h[hs->depth].file_path, all_s->buf + all_s->s[i].sm_filepath))
                    all_s->s[all_s->c_structs].sm_filepath = all_s->s[i].sm_filepath;
            }
            if (all_s->s[all_s->c_structs].sm_filepath == 0)
                write_in_buf(&all_s->s[all_s->c_structs].sm_filepath, hs->h[hs->depth].file_path, all_s->buf, &all_s->len_buf);
            all_s->c_structs++;
            hs->h[hs->depth].comment_s = 1;
            hs->h[hs->depth].s = ST_NONE;
            break;
        default:
            break;
        }
    }
    else
    {
        if ((hs->h[hs->depth].s == ST_FIELD) && (all_s->s[all_s->c_structs].nfields > 0))
        {
            if (all_s->f[all_s->c_fields - 1].sm_comment == 0)
                write_in_buf(&all_s->f[all_s->c_fields - 1].sm_comment, word, all_s->buf, &all_s->len_buf);
        }
        else if (hs->h[hs->depth].comment_s == 1)
        {
            write_in_buf(&all_s->s[all_s->c_structs - 1].sm_comment, word, all_s->buf, &all_s->len_buf);
        }
        hs->h[hs->depth].comment_s = 0;
    }

    free_word(&hs->h[hs->depth]);

    return 0;
}

int read_h_file(all_structs_t *all_s, read_hfiles_t *hs, char *file_dir, char *file_name)
{
    FILE *hfile;
    char file_path[MAX_LEN_FILE_PATH]; // путь

    if (hs->depth >= MAX_DEPTH)
    {
        fprintf(stderr, "ОШИБКА! Достигнуто максимальное возможное количество вложенности (MAX_DEPTH=%d)", MAX_DEPTH);
        exit(1);
    }

    // Открытие файла для считывания
    sprintf(file_path, "%s%s", file_dir, file_name);
    if (!(hfile = fopen(file_path, "rt")))
        error_open_file(file_path);

    memset(&hs->h[hs->depth], 0, sizeof(read_hfile_t));
    hs->h[hs->depth].max_len_word = MAX_STR_SIZE;
    sprintf(hs->h[hs->depth].file_name, file_name);
    sprintf(hs->h[hs->depth].file_path, file_path);

    if (hs->depth == 0)
        printf("Чтение и анализ основного файла [%s]\n", hs->h[hs->depth].file_path);

    free_word(&hs->h[hs->depth]);

    while (fscanf(hfile, "%c", &hs->h[hs->depth].ch) != EOF)
    {
        switch (hs->h[hs->depth].c)
        {
        case COMMENT_NONE: // не в коментарии
            if (isspace(hs->h[hs->depth].ch))
            {
                if ((hs->h[hs->depth].s == ST_FIELD) && (hs->h[hs->depth].f == F_NAME) && (all_s->f[all_s->c_fields].sm_type == 0))
                    continue;
                process_word(all_s, hs, file_dir);
            }
            switch (hs->h[hs->depth].ch)
            {
            case '#':
                hs->h[hs->depth].s = ST_PREPROC;
                break;
            case '/':
                write_in_word(&hs->h[hs->depth]); // запись символа в слово
                hs->h[hs->depth].c = COMMENT_START;
                break;
            case '"':
                if (hs->h[hs->depth].s != ST_INCLUDE_VALUE)
                    write_in_word(&hs->h[hs->depth]); // запись символа в слово
                break;
            case '{':
                if (hs->h[hs->depth].s == ST_TS_OPEN)
                {
                    hs->h[hs->depth].s = ST_FIELD;
                    hs->h[hs->depth].f = F_TYPE;
                }
                else if (hs->h[hs->depth].s == ST_TYPEDEF_STRUCT)
                {
                    process_word(all_s, hs, file_dir);
                    hs->h[hs->depth].s = ST_FIELD;
                    hs->h[hs->depth].f = F_TYPE;
                }
                else
                    write_in_word(&hs->h[hs->depth]); // запись символа в слово
                break;
            case '}':
                if (hs->h[hs->depth].s == ST_FIELD)
                    hs->h[hs->depth].s = ST_TS_CLOSE;
                else
                    write_in_word(&hs->h[hs->depth]); // запись символа в слово
                break;
            case ';':
                if ((hs->h[hs->depth].s == ST_FIELD) && (all_s->f[all_s->c_fields].sm_type == 0))
                {
                    if (hs->h[hs->depth].pz == 1)
                        all_s->f[all_s->c_fields].sm_type = all_s->f[all_s->c_fields - 1].sm_type;
                }
                process_word(all_s, hs, file_dir);
                if ((hs->h[hs->depth].f == F_INDEX) || (hs->h[hs->depth].f == F_NAME))
                    hs->h[hs->depth].f = F_TYPE;
                break;
            case '\n':
                hs->h[hs->depth].comment_s = 0;
                if (hs->h[hs->depth].s == ST_DEFINE_VALUE)
                {
                    process_word(all_s, hs, file_dir);
                    hs->h[hs->depth].s = ST_NONE;
                }
                break;
            case ',':
                if (hs->h[hs->depth].s == ST_FIELD)
                {
                    if (all_s->f[all_s->c_fields].sm_type == 0)
                        all_s->f[all_s->c_fields].sm_type = all_s->f[all_s->c_fields - 1].sm_type;
                    hs->h[hs->depth].f = F_NAME;
                    process_word(all_s, hs, file_dir);
                    hs->h[hs->depth].pz = 1;
                }
                break;
            case '[':
                if ((hs->h[hs->depth].s == ST_FIELD) && (all_s->f[all_s->c_fields].sm_type == 0))
                    all_s->f[all_s->c_fields].sm_type = all_s->f[all_s->c_fields - 1].sm_type;
                process_word(all_s, hs, file_dir);
                hs->h[hs->depth].f = F_INDEX;
                break;
            case ']':
                process_word(all_s, hs, file_dir);
                hs->h[hs->depth].f = F_NAME;
                break;
            case '(':
                if (hs->h[hs->depth].f != F_INDEX)
                    write_in_word(&hs->h[hs->depth]); // запись символа в слово
                break;
            case ')':
                if (hs->h[hs->depth].f != F_INDEX)
                    write_in_word(&hs->h[hs->depth]); // запись символа в слово
                break;
            default:
                write_in_word(&hs->h[hs->depth]); // запись символа в слово
                break;
            }
            break;
        case COMMENT_START: // не в коментарии
            switch (hs->h[hs->depth].ch)
            {
            case '/':
                delete_with_word(&hs->h[hs->depth]); // удаление '/' из коментария
                hs->h[hs->depth].c = COMMENT_NONE;
                process_word(all_s, hs, file_dir);
                hs->h[hs->depth].c = COMMENT_2;
                break;
            case '*':
                delete_with_word(&hs->h[hs->depth]); // удаление '/' из коментария
                hs->h[hs->depth].c = COMMENT_NONE;
                process_word(all_s, hs, file_dir);
                hs->h[hs->depth].c = COMMENT_1;
                break;
            default:
                write_in_word(&hs->h[hs->depth]); // запись символа в слово
                hs->h[hs->depth].c = COMMENT_NONE;
                break;
            }
            break;
        case COMMENT_1: // в комментарии вида /* */
            if (hs->h[hs->depth].ch == '*')
                hs->h[hs->depth].c = COMMENT_1_END;
            write_in_word(&hs->h[hs->depth]); // запись символа в слово
            break;
        case COMMENT_1_END: // в комментарии вида /* */
            switch (hs->h[hs->depth].ch)
            {
            case '/':
                // конец коментария
                delete_with_word(&hs->h[hs->depth]); // удаление последней * из коментария
                process_word(all_s, hs, file_dir);
                hs->h[hs->depth].c = COMMENT_NONE;
                break;
            case '*':
                // продолжаем ждать завершающий /
                write_in_word(&hs->h[hs->depth]); // запись символа в слово
                break;
            default:
                write_in_word(&hs->h[hs->depth]); // запись символа в слово
                hs->h[hs->depth].c = COMMENT_1;
                break;
            }
            break;
        case COMMENT_2: // в комментарии вида //
            if (hs->h[hs->depth].ch == '\n')
            {
                // конец коментария
                process_word(all_s, hs, file_dir);
                hs->h[hs->depth].c = COMMENT_NONE;
            }
            else
                write_in_word(&hs->h[hs->depth]); // запись символа в слово
            break;
        }
    }

    fclose(hfile);
    if (hs->depth == 0)
        printf("Анализ файла [%s] завершен успешно\n\n", hs->h[hs->depth].file_path);
    else
        hs->depth--;

    return 1;
}

int calc_indexes(all_structs_t *all_s)
{
    int nf, nd, i, j;
    char *idx;
    static char ind[128];
    static char name[256];
    int p;

    // цикл по всем полям структур
    for (nf = 0; nf < all_s->c_fields; nf++)
    {
        sprintf(name, all_s->buf + all_s->f[nf].sm_name);
        all_s->f[nf].m_index = all_s->c_idx;

        // посимвольное считывание имени переменной
        for (i = 0; i < strlen(name); i++)
        {
            if (name[i] == '[') // если есть в имени переменной массив
            {
                // запись посимвольно индекса массива
                j = 0;
                while (name[i++] != ']')
                {
                    if (name[i] == ']')
                        break;
                    ind[j++] = name[i];
                }
                ind[j] = 0; // завершение строки

                // если индекс не число, то поиск в define и запись в индекс значение найденного define
                if (atoi(ind) == 0)
                {
                    p = 0;
                    for (nd = 0; nd < all_s->c_define; nd++)
                        if (!strcmp(ind, all_s->buf + all_s->d[nd].sm_name))
                        {
                            all_s->idx[all_s->c_idx++] = all_s->d[nd].value;
                            all_s->f[nf].m_index_count++;
                            p = 1;
                        }
                    if (p == 0)
                    {
                        fprintf(stderr, "ОШИБКА! Найден неизвестный define (%s) в переменной \"%s\"", ind, name);
                        exit(1);
                    }
                }
                else // если число, тогда записываем его
                {
                    all_s->idx[all_s->c_idx++] = atoi(ind);
                    all_s->f[nf].m_index_count++;
                }
            }
        }

        // Удаление индексной части из имени переменной
        if ((idx = strchr(name, '[')))
            idx[0] = 0;
        if (all_s->f[nf].m_index_count != 0)
            rewrite_in_buf(all_s, &all_s->f[nf].sm_name, name);
    }

    return 0;
}

int format_structs(config_t *c, all_structs_t *all_s)
{
    int ns, nf, nd;
    FILE *fp;
    int num = 0;
    static char varname[MAX_STR_SIZE];
    static char id[MAX_STR_SIZE];
    int nm;

    // преобразование индексов массива из имени переменной в значения
    //calc_indexes(all_s);

    if (c->debug)
    {
        //открываем файл на запись и записываем первую строчку
        if (!(fp = fopen(c->file_paths.struct_info, "w+")))
            error_open_file(c->file_paths.struct_info);
        fprintf(fp, "/*\nСчитано define: %d\n", all_s->c_define);
        for (nd = 0; nd < all_s->c_define; nd++)
        {
            fprintf(fp, "%-5d %-30s %d\n", nd, all_s->buf + all_s->d[nd].sm_name, all_s->d[nd].value);
        }
        fprintf(fp, "*/\n//Количество найденных структур (в том числе и все типы): %d\n", all_s->c_structs);
    }

    for (ns = 0; ns < all_s->c_structs; ns++)
    {
        if ((c->debug) && (all_s->s[ns].nfields > 0))
        {
            //подсчитываем количество структур и выводим начальную запись
            num++;
            fprintf(fp, "// номер структуры: %d\ntypedef struct\n{\n", num);
        }

        for (nf = all_s->s[ns].first_field; nf < all_s->s[ns].first_field + all_s->s[ns].nfields; nf++)
        {
            if ((c->debug) && (all_s->s[ns].nfields > 0))
            {
                // печать элемента структуры
                sprintf(varname, all_s->buf + all_s->f[nf].sm_name);
                for (nm = all_s->f[nf].m_index; nm < all_s->f[nf].m_index + all_s->f[nf].m_index_count; nm++)
                {
                    sprintf(id, "[%d]", all_s->idx[nm]);
                    strcat(varname, id);
                }
                strcat(varname, ";");

                if (strlen(all_s->buf + all_s->f[nf].sm_comment) > 1)
                    fprintf(fp, "\t%-10s\t%-20s // %s\n",
                            all_s->buf + all_s->f[nf].sm_type,
                            varname, all_s->buf + all_s->f[nf].sm_comment);
                else
                    fprintf(fp, "\t%-10s\t%-20s\n", all_s->buf + all_s->f[nf].sm_type, varname);
            }
        }
        if ((c->debug) && (all_s->s[ns].nfields > 0))
        {
            if (strlen(all_s->buf + all_s->s[ns].sm_comment) > 1)
                fprintf(fp, "}%s;\t// %s\n\n", all_s->buf + all_s->s[ns].sm_name, all_s->buf + all_s->s[ns].sm_comment);
            else
                fprintf(fp, "}%s;\n\n", all_s->buf + all_s->s[ns].sm_name);
        }
    }
    if (c->debug)
        fclose(fp);

    return 0;
}
