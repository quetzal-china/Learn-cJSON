/*
 * cJSON_path.c - JSON Path 查询实现
 *
 * 基本思路:
 * 1. 将路径字符串解析成链表 (如 $.a.b[0] -> KEY:a -> KEY:b -> INDEX:0)
 * 2. 遍历链表，逐级访问 cJSON 节点
 */

#include "cJSON_path.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* 全局错误信息 */
static char global_path_error[256] = {0};

const char* cJSON_GetPathError(void) {
    return global_path_error;
}

static void set_path_error(const char *msg) {
    strncpy(global_path_error, msg, sizeof(global_path_error) - 1);
    global_path_error[sizeof(global_path_error) - 1] = '\0';
}

/* 创建路径段 */
static cJSON_PathSegment* create_segment(void) {
    cJSON_PathSegment *seg = (cJSON_PathSegment*)malloc(sizeof(cJSON_PathSegment));
    if (seg) {
        memset(seg, 0, sizeof(cJSON_PathSegment));
    }
    return seg;
}

/* 解析单个路径段 */
static cJSON_PathSegment* parse_path_segment(const char **ptr) {
    const char *p = *ptr;
    cJSON_PathSegment *seg = NULL;
    
    /* 处理 .key 形式 */
    if (*p == '.') {
        p++;
        const char *start = p;
        while (*p && *p != '.' && *p != '[' && *p != ']') {
            p++;
        }
        int len = p - start;
        if (len == 0) {
            set_path_error("Empty key after '.'");
            return NULL;
        }
        
        seg = create_segment();
        if (!seg) {
            set_path_error("Memory allocation failed");
            return NULL;
        }
        seg->type = PATH_SEGMENT_KEY;
        seg->value.key = (char*)malloc(len + 1);
        if (!seg->value.key) {
            free(seg);
            set_path_error("Memory allocation failed");
            return NULL;
        }
        strncpy(seg->value.key, start, len);
        seg->value.key[len] = '\0';
    }
    /* 处理 [index] 形式 */
    else if (*p == '[') {
        p++;
        while (*p && isspace(*p)) p++;
        
        if (!isdigit(*p)) {
            set_path_error("Invalid array index");
            return NULL;
        }
        
        int index = 0;
        while (isdigit(*p)) {
            index = index * 10 + (*p - '0');
            p++;
        }
        
        while (*p && isspace(*p)) p++;
        if (*p != ']') {
            set_path_error("Missing ']' in array index");
            return NULL;
        }
        p++;
        
        seg = create_segment();
        if (!seg) {
            set_path_error("Memory allocation failed");
            return NULL;
        }
        seg->type = PATH_SEGMENT_INDEX;
        seg->value.index = index;
    }
    else {
        set_path_error("Invalid path syntax");
        return NULL;
    }
    
    *ptr = p;
    return seg;
}

/* 解析整个路径字符串 */
cJSON_Path* cJSON_ParsePath(const char *path) {
    if (!path || path[0] != '$') {
        set_path_error("Path must start with '$'");
        return NULL;
    }
    
    cJSON_Path *path_obj = (cJSON_Path*)malloc(sizeof(cJSON_Path));
    if (!path_obj) {
        set_path_error("Memory allocation failed");
        return NULL;
    }
    memset(path_obj, 0, sizeof(cJSON_Path));
    
    const char *p = path + 1;
    
    /* 如果只有 $，直接返回 */
    if (*p == '\0') {
        return path_obj;
    }
    
    cJSON_PathSegment *tail = NULL;
    int count = 0;
    
    /* 逐个解析路径段 */
    while (*p) {
        while (*p && isspace(*p)) p++;
        if (*p == '\0') break;
        
        cJSON_PathSegment *seg = parse_path_segment(&p);
        if (!seg) {
            cJSON_DeletePath(path_obj);
            return NULL;
        }
        
        /* 添加到链表尾部 */
        if (!path_obj->segments) {
            path_obj->segments = seg;
            tail = seg;
        } else {
            tail->next = seg;
            tail = seg;
        }
        count++;
    }
    
    path_obj->count = count;
    return path_obj;
}

/* 释放路径对象 */
void cJSON_DeletePath(cJSON_Path *path_obj) {
    if (!path_obj) return;
    
    cJSON_PathSegment *seg = path_obj->segments;
    while (seg) {
        cJSON_PathSegment *next = seg->next;
        if (seg->type == PATH_SEGMENT_KEY) {
            free(seg->value.key);
        }
        free(seg);
        seg = next;
    }
    
    free(path_obj);
}

/* 执行路径查询 */
cJSON* cJSON_ExecutePath(cJSON *root, cJSON_Path *path_obj) {
    if (!root || !path_obj) {
        set_path_error("Invalid arguments");
        return NULL;
    }
    
    cJSON *current = root;
    cJSON_PathSegment *seg = path_obj->segments;
    
    /* 遍历每个路径段 */
    while (seg) {
        if (!current) {
            set_path_error("Path not found");
            return NULL;
        }
        
        if (seg->type == PATH_SEGMENT_KEY) {
            /* 对象键访问 */
            if (!cJSON_IsObject(current)) {
                set_path_error("Expected object for key access");
                return NULL;
            }
            current = cJSON_GetObjectItem(current, seg->value.key);
            if (!current) {
                set_path_error("Key not found");
                return NULL;
            }
        }
        else if (seg->type == PATH_SEGMENT_INDEX) {
            /* 数组索引访问 */
            if (!cJSON_IsArray(current)) {
                set_path_error("Expected array for index access");
                return NULL;
            }
            current = cJSON_GetArrayItem(current, seg->value.index);
            if (!current) {
                set_path_error("Array index out of bounds");
                return NULL;
            }
        }
        
        seg = seg->next;
    }
    
    return current;
}

/* 主接口函数 */
cJSON* cJSON_GetPath(cJSON *root, const char *path) {
    cJSON_Path *path_obj = cJSON_ParsePath(path);
    if (!path_obj) {
        return NULL;
    }
    
    cJSON *result = cJSON_ExecutePath(root, path_obj);
    cJSON_DeletePath(path_obj);
    
    return result;
}

/* 便捷函数 - 直接获取各种类型的值 */
char* cJSON_GetPathString(cJSON *root, const char *path) {
    cJSON *item = cJSON_GetPath(root, path);
    if (!item || !cJSON_IsString(item)) {
        return NULL;
    }
    return item->valuestring;
}

int cJSON_GetPathInt(cJSON *root, const char *path) {
    cJSON *item = cJSON_GetPath(root, path);
    if (!item || !cJSON_IsNumber(item)) {
        return 0;
    }
    return item->valueint;
}

double cJSON_GetPathDouble(cJSON *root, const char *path) {
    cJSON *item = cJSON_GetPath(root, path);
    if (!item || !cJSON_IsNumber(item)) {
        return 0.0;
    }
    return item->valuedouble;
}

int cJSON_GetPathBool(cJSON *root, const char *path, int default_value) {
    cJSON *item = cJSON_GetPath(root, path);
    if (!item || !cJSON_IsBool(item)) {
        return default_value;
    }
    return cJSON_IsTrue(item) ? 1 : 0;
}

/* 格式化输出函数 */
char* cJSON_PrintPathFormatted(cJSON *root, const char *path, const cJSON_PrintOptions *options) {
    cJSON *item = cJSON_GetPath(root, path);
    if (!item) {
        return NULL;
    }
    return cJSON_PrintFormatted(item, options);
}

char* cJSON_PrintPathCompact(cJSON *root, const char *path) {
    cJSON *item = cJSON_GetPath(root, path);
    if (!item) {
        return NULL;
    }
    return cJSON_PrintUnformatted(item);
}