/*
 * test_path_formatted.c - 路径格式化输出测试
 */

#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "cJSON_path.h"

int main(void) {
    printf("=== JSON Path 格式化输出测试 ===\n\n");
    
    const char *json = "{\"server\":{\"host\":\"localhost\",\"port\":8080},"
                       "\"users\":[{\"name\":\"Alice\"}]}";
    
    cJSON *root = cJSON_Parse(json);
    if (!root) {
        printf("解析失败\n");
        return 1;
    }
    
    /* 测试1: 格式化输出对象 */
    printf("$.server:\n");
    cJSON_PrintOptions opts = {' ', 2};
    char *output = cJSON_PrintPathFormatted(root, "$.server", &opts);
    if (output) {
        printf("%s\n\n", output);
        free(output);
    }
    
    /* 测试2: 格式化输出数组元素 */
    printf("$.users[0]:\n");
    output = cJSON_PrintPathFormatted(root, "$.users[0]", NULL);
    if (output) {
        printf("%s\n\n", output);
        free(output);
    }
    
    /* 测试3: 紧凑输出 */
    printf("$.users (紧凑):\n");
    output = cJSON_PrintPathCompact(root, "$.users");
    if (output) {
        printf("%s\n\n", output);
        free(output);
    }
    
    /* 测试4: 单个值 */
    printf("$.server.host:\n");
    output = cJSON_PrintPathFormatted(root, "$.server.host", &opts);
    if (output) {
        printf("%s\n\n", output);
        free(output);
    }
    
    cJSON_Delete(root);
    return 0;
}