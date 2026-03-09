#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== 测试 cJSON_PrintFormatted ===\n\n");

    /* 解析一个复杂的 JSON */
    const char *json_str = "{\"name\":\"John\",\"age\":30,\"is_student\":false,\"courses\":[\"math\",\"english\"],\"address\":{\"city\":\"Beijing\",\"street\":\"Main St\"}}";

    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL) {
        printf("解析失败!\n");
        return 1;
    }

    printf("原始 JSON:\n%s\n\n", json_str);

    /* 测试1: 默认格式 (4空格) */
    printf("=== 测试1: 默认格式 (4空格) ===\n");
    char *output1 = cJSON_PrintFormatted(root, NULL);
    printf("%s\n\n", output1);
    free(output1);

    /* 测试2: 2空格缩进 */
    printf("=== 测试2: 2空格缩进 ===\n");
    cJSON_PrintOptions opts2 = {' ', 2};
    char *output2 = cJSON_PrintFormatted(root, &opts2);
    printf("%s\n\n", output2);
    free(output2);

    /* 测试3: 4空格缩进 */
    printf("=== 测试3: 4空格缩进 ===\n");
    cJSON_PrintOptions opts3 = {' ', 4};
    char *output3 = cJSON_PrintFormatted(root, &opts3);
    printf("%s\n\n", output3);
    free(output3);

    /* 测试4: 制表符缩进 */
    printf("=== 测试4: 制表符缩进 ===\n");
    cJSON_PrintOptions opts4 = {'\t', 1};
    char *output4 = cJSON_PrintFormatted(root, &opts4);
    printf("%s\n\n", output4);
    free(output4);

    /* 测试5: 对比原有 cJSON_Print */
    printf("=== 测试5: 原有 cJSON_Print (制表符缩进) ===\n");
    char *output5 = cJSON_Print(root);
    printf("%s\n\n", output5);
    free(output5);

    /* 清理 */
    cJSON_Delete(root);

    printf("测试完成!\n");
    return 0;
}