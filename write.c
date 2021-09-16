/*
 * write.c
 *
 *  Created on: 25 дек. 2017 г.
 *      Author: Иван
 */

#include "defines.h"
#include "write.h"
#include <sys/types.h>
#include <sys/stat.h>

int get_alignment_s(all_structs_t *all_s, int ns, int *max_a, int config_a)
{
    size_t sz;
    int i, nf;

    for (nf = all_s->s[ns].first_field; nf < all_s->s[ns].first_field + all_s->s[ns].nfields; nf++)
    {
        for (i = 0; i < ns; i++)
        {
            if (!strcmp(all_s->buf + all_s->f[nf].sm_type, all_s->buf + all_s->s[i].sm_name))
            {
                all_s->f[nf].sm_type_std = all_s->s[i].sm_name_std;
                sz = all_s->s[i].size;
                break;
            }
        }
        if (sz == 0)
        {
            fprintf(stderr, "ОШИБКА! Найден неизвестный тип '%s' в структуре '%s' из файла [%s] \n",
                    all_s->buf + all_s->f[nf].sm_type, all_s->buf + all_s->s[i].sm_name, all_s->buf + all_s->s[i].sm_filepath);
            exit(1);
        }

        if (i < all_s->c_std_types)
        {
            if (*max_a < sz)
            {
                if (sz > config_a)
                    *max_a = config_a;
                else
                    *max_a = sz;
            }
        }
        else
        {
            // структура
            get_alignment_s(all_s, i, max_a, config_a);
        }
    }
    return 0;
}

int get_size_and_sm_struct(FILE *fp, all_structs_t *all_s, size_t *total_sz, int ns, int max_a, int depth)
{
    int nf, i, ni;
    size_t sz;
    int count, od;
    char txt_tab[1024];
    static char txt_name[1024];

    sprintf(txt_tab, "\t");
    for (i = 0; i < depth; i++)
        strcat(txt_tab, "\t");

    if (depth == 0)
        fprintf(fp, "typedef struct\n{\n");

    for (nf = all_s->s[ns].first_field; nf < all_s->s[ns].first_field + all_s->s[ns].nfields; nf++)
    {
        sz = 0;
        for (i = 0; i < all_s->c_structs; i++)
        {
            if (all_s->s[i].size != 0)
            {
                if (!strcmp(all_s->buf + all_s->f[nf].sm_type, all_s->buf + all_s->s[i].sm_name))
                {
                    all_s->f[nf].sm_type_std = all_s->s[i].sm_name_std;
                    sz = all_s->s[i].size;
                    break;
                }
            }
        }
        if (sz == 0)
        {
            fprintf(stderr, "ОШИБКА! Найден неизвестный тип '%s' в структуре '%s' из файла [%s] \n",
                    all_s->buf + all_s->f[nf].sm_type, all_s->buf + all_s->s[ns].sm_name, all_s->buf + all_s->s[ns].sm_filepath);
            exit(1);
        }

        // цикл по индексам если они есть
        count = 1;
        for (ni = all_s->f[nf].m_index; ni < all_s->f[nf].m_index + all_s->f[nf].m_index_count; ni++)
            count *= all_s->idx[ni];

        if (max_a == 1)
        {
            // без выравнивания
            if (sz > 1)
            {
                for (ni = 0; ni < count; ni++)
                {

                    if (count > 1)
                        sprintf(txt_name, "%s[%d];", all_s->buf + all_s->f[nf].sm_name, ni);
                    else
                        sprintf(txt_name, "%s;", all_s->buf + all_s->f[nf].sm_name);
                    fprintf(fp, "\t%-10d %s %-30s %-30s \\\\ %s\n", *total_sz, txt_tab, all_s->buf + all_s->f[nf].sm_type, txt_name, all_s->buf + all_s->f[nf].sm_comment);
                    *total_sz += sz;
                }
            }
            else
            {
                sprintf(txt_name, "%s[%d];", all_s->buf + all_s->f[nf].sm_name, count);
                fprintf(fp, "\t%-10d %s %-30s %-30s \\\\ %s\n", *total_sz, txt_tab, all_s->buf + all_s->f[nf].sm_type, txt_name, all_s->buf + all_s->f[nf].sm_comment);
                *total_sz += sz * count;
            }
        }
        else
        {
            if (i < all_s->c_std_types)
            {
                if (sz > 1)
                {
                    for (ni = 0; ni < count; ni++)
                    {
                        od = *total_sz % max_a;
                        if ((*total_sz > 0) && (od != 0) && (sz > 1))
                        {
                            if ((sz == 2) && ((*total_sz % 2) == 1))
                            {
                                *total_sz = *total_sz + 1;
                            }
                            else
                            {
                                if (((sz == 4) && (max_a == 8)) && (*total_sz % 4 == 0))
                                {
                                    //printf("%-30s\n", all_s->buf + all_s->s[ns].sm_name);
                                }
                                else
                                    *total_sz += max_a - od;
                            }
                        }

                        if (count > 1)
                            sprintf(txt_name, "%s[%d];", all_s->buf + all_s->f[nf].sm_name, ni);
                        else
                            sprintf(txt_name, "%s;", all_s->buf + all_s->f[nf].sm_name);

                        fprintf(fp, "\t%-10d %s %-30s %-30s \\\\ %s\n", *total_sz, txt_tab, all_s->buf + all_s->f[nf].sm_type, txt_name, all_s->buf + all_s->f[nf].sm_comment);
                        *total_sz += sz;
                    }
                }
                else
                {
                    od = *total_sz % max_a;
                    if ((*total_sz > 0) && (od != 0) && (sz > 1))
                    {
                        if ((sz == 2) && ((*total_sz % 2) == 1))
                        {
                            *total_sz = *total_sz + 1;
                        }
                        else
                        {
                            if (((sz == 4) && (max_a == 8)) && (*total_sz % 4 == 0))
                            {
                                //printf("%-30s\n", all_s->buf + all_s->s[ns].sm_name);
                            }
                            else
                                *total_sz += max_a - od;
                        }
                    }
                    sprintf(txt_name, "%s[%d];", all_s->buf + all_s->f[nf].sm_name, count);
                    fprintf(fp, "\t%-10d %s %-30s %-30s \\\\ %s\n", *total_sz, txt_tab, all_s->buf + all_s->f[nf].sm_type, txt_name, all_s->buf + all_s->f[nf].sm_comment);
                    *total_sz += sz * count;
                }
            }
            else
            {
                // структура
                for (ni = 0; ni < count; ni++)
                {
                    od = *total_sz % max_a;
                    if ((*total_sz > 0) && (od != 0))
                        *total_sz += max_a - od;

                    if (count > 1)
                        sprintf(txt_name, "%s[%d];", all_s->buf + all_s->f[nf].sm_name, ni);
                    else
                        sprintf(txt_name, "%s;", all_s->buf + all_s->f[nf].sm_name);

                    fprintf(fp, "\t%-10d %s %-30s %-30s \\\\ %s\n", *total_sz, txt_tab, all_s->buf + all_s->f[nf].sm_type, txt_name, all_s->buf + all_s->f[nf].sm_comment);

                    get_size_and_sm_struct(fp, all_s, total_sz, i, max_a, depth + 1);
                }
            }
        }
    }
    if (max_a > 1)
        *total_sz += *total_sz % max_a;
    all_s->s[ns].size = *total_sz;

    if (depth == 0)
    {
        //printf("%d\n", all_s->s[ns].size);
        fprintf(fp, "}%s; %d \\\\ %d   \\\\ %s \n\n", all_s->buf + all_s->s[ns].sm_name, max_a, all_s->s[ns].size, all_s->buf + all_s->s[ns].sm_comment);
    }

    return 0;
}

int get_size_and_sm(config_t *c, all_structs_t *all_s)
{
    int ns, max_a;
    size_t total_sz;
    FILE *fp;

    if (!(fp = fopen(c->file_paths.par, "wt")))
        error_open_file(c->file_paths.par);

    for (ns = 0; ns < all_s->c_structs; ns++)
    {
        if (all_s->s[ns].size != 0)
            continue;

        max_a = 1;
        total_sz = 0;

        if (c->alignment != 0)
            get_alignment_s(all_s, ns, &max_a, c->alignment);

        get_size_and_sm_struct(fp, all_s, &total_sz, ns, max_a, 0);
    }
    fclose(fp);

    return 0;
}

int write_bdtypes(config_t *c, all_structs_t *all_s, all_types_t *all_t)
{
    FILE *read_types;
    FILE *bd_types;
    int ns, num, count, id;
    static char write_str[MAX_STR_SIZE];
    static char name[128];
    int i, write_t;
    int ret;

    memset(all_t, 0, sizeof(all_types_t));

    /*открываем STRUCT_PATH на чтение и BDTYPE_PATH на запись*/
    if (!(read_types = fopen(c->file_paths.structn, "rt")))
        error_open_file(c->file_paths.structn);

    if (!(bd_types = fopen(c->file_paths.bd_types, "w")))
        error_open_file(c->file_paths.bd_types);

    printf("Чтение файла [%s] и запись в [%s]\n", c->file_paths.structn, c->file_paths.bd_types);

    // запись стандартных типов
    for (ns = 0; ns < all_s->c_structs; ns++)
    {
        if (all_s->s[ns].nfields < 1)
        {
            all_t->t[all_t->count].sm_name = all_s->s[ns].sm_name;
            all_t->t[all_t->count].num = all_s->s[ns].size;
            all_t->t[all_t->count].id = all_s->s[ns].typeid;

            write_t = 1;
            for (i = 0; i < all_t->count; i++)
                if (all_t->t[all_t->count].id == all_t->t[i].id)
                    write_t = 0;

            if ((c->write_types == 1) && (write_t == 1))
            {
                sprintf(write_str, "%-20d %-6d\t%s",
                        all_t->t[all_t->count].id,
                        all_t->t[all_t->count].num,
                        all_s->buf + all_t->t[all_t->count].sm_name);
                if (c->debug)
                    printf("%-80s OK\n", write_str);
                strcat(write_str, "\n");
                fwrite(write_str, 1, strlen(write_str), bd_types);
            }
            all_t->count++;
        }
    }

    count = all_t->count;

    // чтение  файла
    do
    {
        // считывание имени типа
        fgets(write_str, MAX_STR_SIZE - 1, read_types);
        if (strlen(write_str) < 2)
            continue;
        ret = sscanf(write_str, "%s %d %d", name, &num, &id);
        if ((name[0] == '#') || (name == 0) || (ret != 3))
            continue;

        // поиск типа в структурах
        for (ns = 0; ns < all_s->c_structs; ns++)
        {
            if (!strcmp(name, all_s->buf + all_s->s[ns].sm_name))
            {
                if (!id)
                    id = all_t->count + c->first_num_type - count;

                if (num != 1)
                    sprintf(name, "%s[%d]", name, num);
                sprintf(write_str, "%-20d %-6d\t%s",
                        id,
                        num * all_s->s[ns].size,
                        name);
                if (c->debug)
                    printf("%-80s OK\n", write_str);
                strcat(write_str, "\n");
                fwrite(write_str, 1, strlen(write_str), bd_types);

                write_in_buf(&all_t->t[all_t->count].sm_name, name, all_s->buf, &all_s->len_buf);
                all_t->t[all_t->count].num = num;
                all_t->t[all_t->count].id = id;
                break;
            }
        }
        if (all_t->t[all_t->count].id == 0)
        {
            fprintf(stderr, "ОШИБКА! Структура %s не найдена\n", name);
            exit(1);
        }
        all_t->count++;
    } while (!feof(read_types));

    fclose(read_types);
    fclose(bd_types);
    printf("OK\n\n");

    return 0;
}

int chk_id(bd_data_t *bd, int id, char *file_path, FILE *fp)
{

    int nid;

    //проверка ид в массиве
    for (nid = 0; nid < bd->count_id; nid++)
        if (id == bd->array_id[nid])
        {
            printf("ОШИБКА! Повторяющийся id=%d\n", id);
            fclose(fp);
            //unlink(file_path);
            exit(1);
        }
    bd->array_id[bd->count_id++] = id; // запись ид в массив

    return 0;
}

int check_name_struct(all_structs_t *all_s, char *struct_name, int *n_struct)
{
    int ns;
    for (ns = 0; ns < all_s->c_structs; ns++)
    {
        if (!strcmp(all_s->buf + all_s->s[ns].sm_name, struct_name))
        {
            *n_struct = ns;
            return 0;
        }
    }
    return -1;
}

int check_struct_name_in_bdtypes(all_structs_t *all_s, all_types_t *all_t, char *name, int *type)
{
    int nt;

    for (nt = 0; nt < all_t->count; nt++)
    {
        if (!strcmp(all_s->buf + all_t->t[nt].sm_name, name))
        {
            *type = nt;
            return 0;
        }
    }

    return -1;
}

int get_type(all_structs_t *all_s, all_types_t *all_t, int nf, int *all_cycles, int num_index, char *typename)
{
    int i, si, ci, ei;
    static char idx[16];
    int ret;
    int nt;

    nt = -1;

    sprintf(typename, "%s", all_s->buf + all_s->f[nf].sm_type);
    si = all_s->f[nf].m_index;
    ci = all_s->f[nf].m_index_count;
    ei = si + ci;

    for (i = si + num_index; i < ei; i++)
    {
        sprintf(idx, "[%d]", all_s->idx[i]);
        strcat(typename, idx);
        if (num_index == 0)
            *all_cycles = *all_cycles * all_s->idx[i];
    }

    if (num_index > 0)
    {
        *all_cycles = 1;
        for (i = si; i < ei - ci + num_index; i++)
        {
            *all_cycles = *all_cycles * all_s->idx[i];
        }
    }

    ret = check_struct_name_in_bdtypes(all_s, all_t, typename, &nt);
    if (ret < 0)
    {
        if (si + num_index < ei)
        {
            sprintf(typename, "%s[%d]", all_s->buf + all_s->f[nf].sm_type, *all_cycles);
            ret = check_struct_name_in_bdtypes(all_s, all_t, typename, &nt);
        }
        if (ret < 0)
        {
            if (ci == num_index)
                return nt;
            if (ci > num_index)
                nt = get_type(all_s, all_t, nf, all_cycles, ++num_index, typename);
        }
    }

    if ((num_index == 0) && (ret == 0))
        *all_cycles = 1;

    return nt;
}

int write_bdcreate(config_t *c, all_structs_t *all_s, all_types_t *all_t, bd_data_t *bd, int state, int nt, int nf, char *name)
{
    FILE *fp;
    char file_path[1024];
    char table_name[256];
    char str[1024];

    if (bd->rv.type_bd == 1)
    {
        if (bd->depth == 0)
            sprintf(table_name, "%s", bd->rv.prefix);
        else
        {
            sprintf(table_name, "%s_%s", bd->rv.prefix, all_s->buf + all_s->f[bd->di[bd->depth].nf].sm_name);
        }

        if (state != END)
        {
            sprintf(file_path, "%sCreateBD.%d.tmp", TMP_PATH, bd->depth);
            // откритие файла на запись
            if (!(fp = fopen(file_path, "a")))
            {
                fprintf(stderr, "Ошибка при создинии временного файла [%s]\n", file_path);
                exit(1);
            }
            fseek(fp, 0, SEEK_END);
        }

        switch (state)
        {
        case START_QUERY:
            // начальная запись в файл
            fprintf(fp, "CREATE SEQUENCE %s_id;\n"
                        "CREATE TABLE %s (\n"
                        "id integer PRIMARY KEY DEFAULT nextval('%s_id'),\n"
                        "%-12s\t%s,\n"
                        "%-12s\t%s",
                    table_name, table_name, table_name, "date_time", "timestamp", "n", "integer");
            break;
        case IN_QUERY:
        {
            static char varname[256];
            int ns, ni;

            if (all_t->t[nt].id >= 1000000)
                fprintf(fp, ",\n%-12s\t%s", all_s->buf + all_s->f[nf].sm_name, "text");
            else if ((all_t->t[nt].id >= 100000) || (nt < all_s->c_std_types))
            {
                // как массив
                check_name_struct(all_s, all_s->buf + all_s->f[nf].sm_type, &ns);
                sprintf(varname, all_s->buf + all_s->s[ns].sm_type_bd);
                for (ni = all_s->f[nf].m_index; ni < all_s->f[nf].m_index + all_s->f[nf].m_index_count; ni++)
                {
                    char idx[32];

                    sprintf(idx, "[%d]", all_s->idx[ni]);
                    strcat(varname, idx);
                }
                fprintf(fp, ",\n%-12s\t%s", all_s->buf + all_s->f[nf].sm_name, varname);
            }
            else
                // как bytea
                fprintf(fp, ",\n%-12s\t%s", name, "bytea");
            break;
        }
        case END_QUERY:
        {
            fprintf(fp, "\n);\n\n");
            fclose(fp);

            // откритие файла на запись
            if (!(fp = fopen(c->file_paths.bd_drop, "a")))
            {
                fprintf(stderr, "Ошибка при создинии файла [%s]\n", c->file_paths.bd_drop);
                exit(1);
            }
            fprintf(fp, "DROP TABLE %s;\n", table_name);
            fprintf(fp, "DROP SEQUENCE %s_id;\n", table_name);
            break;
        }
        case END:
        {
            FILE *bdcreate;

            if (!(bdcreate = fopen(c->file_paths.bd_create, "at")))
            {
                fprintf(stderr, "ОШИБКА! Файл [%s] не создался\n", c->file_paths.bd_create);
                exit(1);
            }
            for (; bd->max_depth >= 0; bd->max_depth--)
            {

                sprintf(file_path, "%sCreateBD.%d.tmp", TMP_PATH, bd->max_depth);
                if ((fp = fopen(file_path, "r")))
                {
                    while (!feof(fp))
                    {
                        fgets(str, MAX_STR_SIZE - 1, fp);
                        if (feof(fp))
                            break;
                        fprintf(bdcreate, "%s", str);
                    }
                    fclose(fp);
                    unlink(file_path);
                }
                else
                {
                    //fprintf(stderr,	"ОШИБКА! Невозможно прочитать временный файл [%s]\n", file_path);
                    //unlink(c->file_paths.bd_create);
                    //exit(1);
                }
            }
            fclose(bdcreate);
            break;
        }
        default:
            break;
        }
        if (state != END)
            fclose(fp);
    }

    return 0;
}

int write_struct_in_bd_data(config_t *c, all_structs_t *all_s, all_types_t *all_t, bd_data_t *bd, FILE *bddata)
{
    int ns, nf, nt;
    int i, j, all_cycles;
    unsigned int id[MAX_COUNT_ID_STRUCT];
    unsigned int count_id;
    char typename[256];
    char varname[256];

    if (bd->max_depth < bd->depth)
        bd->max_depth = bd->depth;

    // проверка на максимально допустимый уровень рекурсии
    if (bd->depth >= MAX_DEPTH_STRUCT)
    {
        fprintf(stderr, "Превышен допустимый уровень вложенности (MAX_DEPTH_STRUCT=%d)\n", MAX_DEPTH_STRUCT);
        exit(1);
    }

    if (bd->depth == 0)
        printf("Запись структуры %-40s", bd->rv.struct_name);
    else if (bd->max_depth < bd->depth)
        bd->max_depth = bd->depth;

    ns = bd->di[bd->depth].ns;
    count_id = 0;

    write_bdcreate(c, all_s, all_t, bd, START_QUERY, -1, -1, "");

    if ((check_struct_name_in_bdtypes(all_s, all_t, all_s->buf + all_s->s[ns].sm_name, &nt)) >= 0)
    {
        chk_id(bd, bd->rv.id, c->file_paths.bd_data, bddata);
        fprintf(bddata, "%-10d %-6d %-5d %-25d %-20s %-15s %s\n",
                bd->rv.id++,
                all_t->t[nt].id,
                bd->rv.type_bd,
                0,
                bd->rv.prefix,
                bd->rv.table_name,
                all_s->buf + all_s->s[ns].sm_comment);
        write_bdcreate(c, all_s, all_t, bd, IN_QUERY, nt, all_s->c_fields, all_s->buf + all_s->s[ns].sm_name);
        printf(" OK\n");
        bd->cur_id = bd->rv.id;
        return 0;
    }
    else
    {
        // запись первого ид (для записи этого ид в итоговой строке)
        chk_id(bd, bd->rv.id, c->file_paths.bd_data, bddata);
        bd->di[bd->depth].first_id = bd->rv.id++;

        // цикл по полям структуры
        for (nf = all_s->s[ns].first_field; nf < all_s->s[ns].first_field + all_s->s[ns].nfields; nf++)
        {
            all_cycles = 1;
            nt = get_type(all_s, all_t, nf, &all_cycles, 0, typename);
            check_name_struct(all_s, all_s->buf + all_s->f[nf].sm_type, &bd->di[bd->depth].ns);

            if (nt > 0)
            {
                for (i = 0; i < all_cycles; i++)
                {
                    if ((all_s->s[ns].nfields == 1) && (all_cycles == 1))
                    {
                        bd->rv.id--;
                    }
                    else
                    {
                        chk_id(bd, bd->rv.id, c->file_paths.bd_data, bddata);
                        id[count_id++] = bd->rv.id;
                    }

                    if (all_cycles == 1)
                        sprintf(varname, "%s", all_s->buf + all_s->f[nf].sm_name);
                    else
                        sprintf(varname, "%s%d", all_s->buf + all_s->f[nf].sm_name, i);
                    fprintf(bddata, "%-10d %-6d %-5d %-25d %-30s %-20s %-40s // %s\n",
                            bd->rv.id++,
                            all_t->t[nt].id,
                            bd->rv.type_bd,
                            0,
                            bd->rv.prefix,
                            varname,
                            typename,
                            all_s->buf + all_s->f[nf].sm_comment);
                    write_bdcreate(c, all_s, all_t, bd, IN_QUERY, nt, nf, varname);
                }
            }
            else
            {
                if ((all_s->s[bd->di[bd->depth].ns].nfields > 0) && (all_t->t[nt].id < c->first_num_type))
                {
                    for (i = 0; i < all_cycles; i++)
                    {
                        bd->depth++;
                        memset(&bd->di[bd->depth], 0, sizeof(depth_info_t));
                        bd->di[bd->depth].nf = nf;
                        bd->di[bd->depth].ns = bd->di[bd->depth - 1].ns;
                        write_struct_in_bd_data(c, all_s, all_t, bd, bddata);
                        id[count_id++] = bd->di[bd->depth + 1].first_id;
                    }
                }
                else
                {
                    fprintf(stderr, "ОШИБКА! Структура '%s' не найдена в типах БД\n", all_s->buf + all_s->f[nf].sm_type);
                    fclose(bddata);
                    unlink(c->file_paths.bd_data);
                    exit(1);
                }
            }
        }
    }

    if ((all_s->s[ns].nfields > 1) || (all_cycles > 1) || (bd->max_depth > bd->depth))
    {
        fprintf(bddata, "%-10d %-6d %-5d %-4d ", bd->di[bd->depth].first_id, 0, bd->rv.type_bd, count_id);
        j = 0;
        for (i = 0; i < count_id; i++)
        {
            fprintf(bddata, "%d ", id[i]);
            if (!(++j % 10))
                fprintf(bddata, "\n                             ");
        }
    }

    write_bdcreate(c, all_s, all_t, bd, END_QUERY, -1, -1, "");
    if (bd->depth == 0)
    {
        printf(" OK\n");
        if (count_id > 0)
            fprintf(bddata, "\t%s \t%s \t%s\n", bd->rv.prefix, bd->rv.table_name, all_s->buf + all_s->s[ns].sm_comment);
        write_bdcreate(c, all_s, all_t, bd, END, -1, -1, "");
    }
    else
    {

        if (count_id > 0)
            fprintf(bddata, "\t%s \t%s \t%s\n", bd->rv.prefix, all_s->buf + all_s->f[bd->di[bd->depth].nf].sm_name, all_s->buf + all_s->f[bd->di[bd->depth].nf].sm_comment);
        bd->depth--;
    }

    bd->cur_id = bd->rv.id;

    return 0;
}

int read_config_and_write_files(config_t *c, all_structs_t *all_s, all_types_t *all_t, FILE *cfg)
{
    FILE *bddata;
    static char str[MAX_STR_SIZE];
    bd_data_t bd;
    int ret;
    static struct stat st;

    memset(&bd, 0, sizeof(bd_data_t));

    printf("Создание файла [%s]\n", c->file_paths.bd_data);

    // открытие файла на запись
    unlink(c->file_paths.bd_data);
    if (!(bddata = fopen(c->file_paths.bd_data, "a")))
        error_open_file(c->file_paths.bd_data);
    unlink(c->file_paths.bd_create);

    bd.cur_id = 0;
    //считываем файл по строкам
    while (!feof(cfg))
    {
        // считыванеи строки и проверка на конец файла
        fgets(str, MAX_STR_SIZE - 1, cfg);
        if (feof(cfg))
            break;

        trim(str); // удаление пробелов сначала и конца строки

        // проверка строки на специальный символ
        if (str[0] != c->symbol)
        {
            fprintf(bddata, "%s", str);
            continue;
        }
        else
            str[0] = ' ';

        // считывание значений из строки
        sscanf(str, "%d%d%s%s%s", &bd.rv.id, &bd.rv.type_bd, bd.rv.prefix, bd.rv.table_name, bd.rv.struct_name);

        //проверка на нулевой ид
        if (!bd.rv.id)
            bd.rv.id = bd.cur_id;
        //else
        //	chk_id(&bd, bd.rv.id, c->file_paths.bd_data);

        memset(&bd.di[bd.depth], 0, sizeof(depth_info_t));
        bd.max_depth = 0;
        ret = check_name_struct(all_s, bd.rv.struct_name, &bd.di[bd.depth].ns);

        if (ret == -1)
        {
            fprintf(stderr, "ОШИБКА! Структура '%s' не найдена\n", bd.rv.struct_name);
            fclose(bddata);
            unlink(c->file_paths.bd_data);
            exit(1);
        }
        write_struct_in_bd_data(c, all_s, all_t, &bd, bddata);
    }

    fclose(bddata);

    stat(c->file_paths.bd_create, &st);
    if (st.st_size == 0)
        unlink(c->file_paths.bd_create);

    return 0;
}
