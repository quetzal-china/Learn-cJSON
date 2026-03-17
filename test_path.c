/*
 * test_path.c - JSON Path 功能测试
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "cJSON_path.h"

static int total_tests = 0;
static int passed_tests = 0;

#define TEST(expr, desc) do { \
    total_tests++; \
    if (expr) { \
        printf("[OK] %s\n", desc); \
        passed_tests++; \
    } else { \
        printf("[FAIL] %s\n", desc); \
    } \
} while(0)

int main(void) {
    printf("=== JSON Path 测试 ===\n\n");
    
    /* 测试1: 基本对象访问 */
    {
        printf("--- 基本对象访问 ---\n");
        const char *json = "{\"name\":\"John\",\"age\":30}";
        cJSON *root = cJSON_Parse(json);
        
        cJSON *name = cJSON_GetPath(root, "$.name");
        TEST(name && cJSON_IsString(name), "$.name 存在且为字符串");
        TEST(strcmp(name->valuestring, "John") == 0, "值为 'John'");
        
        cJSON *age = cJSON_GetPath(root, "$.age");
        TEST(age && cJSON_IsNumber(age), "$.age 存在且为数字");
        TEST(age->valueint == 30, "值为 30");
        
        cJSON_Delete(root);
        printf("\n");
    }
    
    /* 测试2: 嵌套对象 */
    {
        printf("--- 嵌套对象访问 ---\n");
        const char *json = "{\"server\":{\"host\":\"localhost\",\"port\":8080}}";
        cJSON *root = cJSON_Parse(json);
        
        cJSON *host = cJSON_GetPath(root, "$.server.host");
        TEST(host && strcmp(host->valuestring, "localhost") == 0, 
             "$.server.host = localhost");
        
        cJSON *port = cJSON_GetPath(root, "$.server.port");
        TEST(port && port->valueint == 8080, "$.server.port = 8080");
        
        cJSON_Delete(root);
        printf("\n");
    }
    
    /* 测试3: 数组访问 */
    {
        printf("--- 数组访问 ---\n");
        const char *json = "[10, 20, 30]";
        cJSON *root = cJSON_Parse(json);
        
        cJSON *item0 = cJSON_GetPath(root, "$[0]");
        TEST(item0 && item0->valueint == 10, "$[0] = 10");
        
        cJSON *item2 = cJSON_GetPath(root, "$[2]");
        TEST(item2 && item2->valueint == 30, "$[2] = 30");
        
        cJSON_Delete(root);
        printf("\n");
    }
    
    /* 测试4: 混合访问 */
    {
        printf("--- 混合访问 ---\n");
        const char *json = "{\"items\":[{\"id\":1},{\"id\":2}]}";
        cJSON *root = cJSON_Parse(json);
        
        cJSON *id0 = cJSON_GetPath(root, "$.items[0].id");
        TEST(id0 && id0->valueint == 1, "$.items[0].id = 1");
        
        cJSON *id1 = cJSON_GetPath(root, "$.items[1].id");
        TEST(id1 && id1->valueint == 2, "$.items[1].id = 2");
        
        cJSON_Delete(root);
        printf("\n");
    }
    
    /* 测试5: 便捷函数 */
    {
        printf("--- 便捷函数 ---\n");
        const char *json = "{\"name\":\"Alice\",\"age\":25,\"active\":true}";
        cJSON *root = cJSON_Parse(json);
        
        char *name = cJSON_GetPathString(root, "$.name");
        TEST(name && strcmp(name, "Alice") == 0, "GetString 正常");
        
        int age = cJSON_GetPathInt(root, "$.age");
        TEST(age == 25, "GetInt 正常");
        
        int active = cJSON_GetPathBool(root, "$.active", 0);
        TEST(active == 1, "GetBool 正常");
        
        cJSON_Delete(root);
        printf("\n");
    }
    
    /* 测试6: 错误处理 */
    {
        printf("--- 错误处理 ---\n");
        const char *json = "{\"data\":123}";
        cJSON *root = cJSON_Parse(json);
        
        cJSON *invalid = cJSON_GetPath(root, "invalid");
        TEST(invalid == NULL, "无效路径返回 NULL");
        
        cJSON *nonexist = cJSON_GetPath(root, "$.notexist");
        TEST(nonexist == NULL, "不存在的键返回 NULL");
        
        cJSON_Delete(root);
        printf("\n");
    }
    
    printf("===================\n");
    printf("总计: %d/%d 通过\n", passed_tests, total_tests);
    printf("===================\n");
    
    return (passed_tests == total_tests) ? 0 : 1;
}