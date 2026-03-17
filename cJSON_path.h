/*
 * cJSON_path.h - JSON Path 查询功能
 *
 * 支持通过路径表达式访问 cJSON 节点
 * 例如: $.store.book[0].title
 *
 * 2026-03-17
 */

#ifndef CJSON_PATH_H
#define CJSON_PATH_H

#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 路径段类型 */
typedef enum {
    PATH_SEGMENT_KEY,    /* 对象键名 */
    PATH_SEGMENT_INDEX   /* 数组索引 */
} cJSON_PathSegmentType;

/* 路径段结构 - 用链表存储路径的每一部分 */
typedef struct cJSON_PathSegment {
    cJSON_PathSegmentType type;
    union {
        char *key;       /* 键名 */
        int index;       /* 数组下标 */
    } value;
    struct cJSON_PathSegment *next;
} cJSON_PathSegment;

/* 解析后的路径对象 */
typedef struct {
    cJSON_PathSegment *segments;
    int count;
    char *error;
} cJSON_Path;

/* 主接口函数 */
cJSON* cJSON_GetPath(cJSON *root, const char *path);
char* cJSON_GetPathString(cJSON *root, const char *path);
int cJSON_GetPathInt(cJSON *root, const char *path);
double cJSON_GetPathDouble(cJSON *root, const char *path);
int cJSON_GetPathBool(cJSON *root, const char *path, int default_value);

/* 格式化输出函数 */
char* cJSON_PrintPathFormatted(cJSON *root, const char *path, const cJSON_PrintOptions *options);
char* cJSON_PrintPathCompact(cJSON *root, const char *path);

/* 内部函数 */
cJSON_Path* cJSON_ParsePath(const char *path);
void cJSON_DeletePath(cJSON_Path *path_obj);
cJSON* cJSON_ExecutePath(cJSON *root, cJSON_Path *path_obj);

/* 错误处理 */
const char* cJSON_GetPathError(void);

#ifdef __cplusplus
}
#endif

#endif