/*
 * json_formatter.c - JSON 格式化命令行工具
 *
 * 用法示例:
 *   json_formatter input.json
 *   json_formatter -i 2 input.json
 *   json_formatter -p '$.name' input.json
 *   cat input.json | json_formatter
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "cJSON.h"
#include "cJSON_path.h"

#define VERSION "1.0"
#define MAX_FILE_SIZE (10 * 1024 * 1024)  /* 最大 10MB */

typedef struct {
    char indent_char;
    int indent_count;
    int use_tabs;
    int compact;
    int sort_keys;
    int ensure_ascii;
    int trailing_newline;
    char *input_file;
    char *output_file;
    char *path_query;
    int show_help;
    int show_version;
} Options;

static void print_help(const char *prog_name) {
    printf("用法: %s [选项] [输入文件]\n\n", prog_name);
    printf("选项:\n");
    printf("  -i <数字>    缩进空格数 (默认: 4)\n");
    printf("  -t           使用制表符缩进\n");
    printf("  -c           紧凑输出\n");
    printf("  -s           按键名排序\n");
    printf("  -o <文件>    输出文件\n");
    printf("  -p <路径>    JSON Path 查询\n");
    printf("  -h           显示帮助\n");
    printf("  -v           显示版本\n");
}

static char* read_file(FILE *fp) {
    /* 处理标准输入 */
    if (fp == stdin) {
        size_t capacity = 4096;
        size_t len = 0;
        char *buffer = (char*)malloc(capacity);
        if (!buffer) {
            fprintf(stderr, "内存分配失败\n");
            return NULL;
        }
        
        int c;
        while ((c = fgetc(fp)) != EOF) {
            if (len + 1 >= capacity) {
                capacity *= 2;
                if (capacity > MAX_FILE_SIZE) {
                    fprintf(stderr, "输入太大\n");
                    free(buffer);
                    return NULL;
                }
                char *new_buffer = (char*)realloc(buffer, capacity);
                if (!new_buffer) {
                    free(buffer);
                    return NULL;
                }
                buffer = new_buffer;
            }
            buffer[len++] = (char)c;
        }
        buffer[len] = '\0';
        return buffer;
    }
    
    /* 处理文件 */
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (file_size > MAX_FILE_SIZE) {
        fprintf(stderr, "文件太大\n");
        return NULL;
    }
    
    char *buffer = (char*)malloc(file_size + 1);
    if (!buffer) {
        return NULL;
    }
    
    size_t read_size = fread(buffer, 1, file_size, fp);
    buffer[read_size] = '\0';
    return buffer;
}

static int parse_options(int argc, char *argv[], Options *opts) {
    memset(opts, 0, sizeof(Options));
    opts->indent_char = ' ';
    opts->indent_count = 4;
    opts->trailing_newline = 1;
    
    int opt;
    while ((opt = getopt(argc, argv, "i:tcso:p:hv")) != -1) {
        switch (opt) {
            case 'i':
                opts->indent_count = atoi(optarg);
                break;
            case 't':
                opts->use_tabs = 1;
                opts->indent_char = '\t';
                opts->indent_count = 1;
                break;
            case 'c':
                opts->compact = 1;
                break;
            case 's':
                opts->sort_keys = 1;
                break;
            case 'o':
                opts->output_file = optarg;
                break;
            case 'p':
                opts->path_query = optarg;
                break;
            case 'h':
                opts->show_help = 1;
                return 0;
            case 'v':
                opts->show_version = 1;
                return 0;
        }
    }
    
    if (optind < argc) {
        opts->input_file = argv[optind];
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    Options opts;
    
    if (parse_options(argc, argv, &opts) != 0) {
        return 1;
    }
    
    if (opts.show_help) {
        print_help(argv[0]);
        return 0;
    }
    
    if (opts.show_version) {
        printf("json_formatter v%s\n", VERSION);
        return 0;
    }
    
    /* 打开输入 */
    FILE *input_fp = stdin;
    if (opts.input_file) {
        input_fp = fopen(opts.input_file, "r");
        if (!input_fp) {
            fprintf(stderr, "无法打开文件: %s\n", opts.input_file);
            return 1;
        }
    }
    
    /* 读取内容 */
    char *json_str = read_file(input_fp);
    if (!json_str) {
        if (input_fp != stdin) fclose(input_fp);
        return 1;
    }
    
    /* 解析 JSON */
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        fprintf(stderr, "JSON 解析失败\n");
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            fprintf(stderr, "错误位置: %s\n", error_ptr);
        }
        free(json_str);
        if (input_fp != stdin) fclose(input_fp);
        return 1;
    }
    
    /* 路径查询 */
    cJSON *target = root;
    if (opts.path_query) {
        target = cJSON_GetPath(root, opts.path_query);
        if (!target) {
            fprintf(stderr, "路径查询失败: %s\n", cJSON_GetPathError());
            cJSON_Delete(root);
            free(json_str);
            if (input_fp != stdin) fclose(input_fp);
            return 1;
        }
    }
    
    /* 格式化输出 */
    char *output_str = NULL;
    if (opts.compact) {
        output_str = cJSON_PrintUnformatted(target);
    } else {
        cJSON_PrintOptions print_opts = {
            .indent_char = opts.indent_char,
            .indent_count = opts.indent_count,
            .sort_keys = opts.sort_keys,
            .compact_arrays = 0,
            .compact_objects = 0,
            .trailing_newline = opts.trailing_newline,
            .ensure_ascii = opts.ensure_ascii,
            .space_after_colon = 1,
            .space_after_comma = 1
        };
        output_str = cJSON_PrintFormatted(target, &print_opts);
    }
    
    if (!output_str) {
        fprintf(stderr, "格式化失败\n");
        cJSON_Delete(root);
        free(json_str);
        if (input_fp != stdin) fclose(input_fp);
        return 1;
    }
    
    /* 输出结果 */
    FILE *output_fp = stdout;
    if (opts.output_file) {
        output_fp = fopen(opts.output_file, "w");
        if (!output_fp) {
            fprintf(stderr, "无法创建文件: %s\n", opts.output_file);
            free(output_str);
            cJSON_Delete(root);
            free(json_str);
            if (input_fp != stdin) fclose(input_fp);
            return 1;
        }
    }
    
    fprintf(output_fp, "%s", output_str);
    if (opts.trailing_newline && !opts.compact) {
        fprintf(output_fp, "\n");
    }
    
    /* 清理 */
    if (output_fp != stdout) fclose(output_fp);
    if (input_fp != stdin) fclose(input_fp);
    
    free(output_str);
    cJSON_Delete(root);
    free(json_str);
    
    return 0;
}