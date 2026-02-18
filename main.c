#if 0
// 之前的简单测试代码
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
int main(void)
{
    const char *json_string = "{\"name\":\"John\",\"age\":30,\"is_student\":false,\"courses\":[\"math\",\"english\"]}";
    
    printf("JSON String: %s\n\n", json_string);
    
    cJSON *root = cJSON_Parse(json_string);
    if (root == NULL)
    {
        printf("Error: Failed to parse JSON\n");
        return 1;
    }
    
    cJSON *name = cJSON_GetObjectItem(root, "name");
    cJSON *age = cJSON_GetObjectItem(root, "age");
    cJSON *is_student = cJSON_GetObjectItem(root, "is_student");
    cJSON *courses = cJSON_GetObjectItem(root, "courses");
    
    printf("Name: %s\n", cJSON_GetStringValue(name));
    printf("Age: %f\n", cJSON_GetNumberValue(age));
    printf("Is Student: %s\n", cJSON_IsTrue(is_student) ? "true" : "false");
    
    printf("Courses: ");
    cJSON *course = NULL;
    cJSON_ArrayForEach(course, courses)
    {
        printf("%s ", cJSON_GetStringValue(course));
    }
    printf("\n");
    
    cJSON_Delete(root);
    
    return 0;
}
#endif

#if 0
// 简单测试 cJSON_CreateNull
#include "cJSON.h"
#include <stdio.h>
int main(void)
{
    printf("=== 测试 cJSON_CreateNull ===\n");
    
    cJSON *null_item = cJSON_CreateNull();
    
    if (null_item != NULL) {
        printf("创建成功！\n");
        printf("类型值: %d\n", null_item->type);
        printf("cJSON_NULL = %d\n", cJSON_NULL);
    }
    
    cJSON_Delete(null_item);
    return 0;
}
#endif

#if 0
// 复杂 JSON 结构，测试递归删除
#include "cJSON.h"
#include <stdio.h>
int main(void)
{
    // 复杂嵌套结构：
    // {
    //     "name": "test",
    //     "items": [
    //         {"key": "value1"},
    //         {"key": "value2"}
    //     ],
    //     "nested": {
    //         "inner": "data"
    //     }
    // }
    
    const char *json_string = "{\"name\":\"test\",\"items\":[{\"key\":\"value1\"},{\"key\":\"value2\"}],\"nested\":{\"inner\":\"data\"}}";
    
    printf("=== 测试递归删除 ===\n");
    printf("JSON: %s\n\n", json_string);
    
    cJSON *root = cJSON_Parse(json_string);
    if (root == NULL)
    {
        printf("解析失败！\n");
        return 1;
    }
    
    printf("解析成功！\n");
    printf("根节点地址: %p\n", (void*)root);
    
    // 在这里设置断点调试 cJSON_Delete
    cJSON_Delete(root);
    printf("删除完成！\n");
    
    return 0;
}
#endif

#if 0
// 测试 cJSON_CreateString
#include "cJSON.h"
#include <stdio.h>
int main(void)
{
    printf("=== 测试 cJSON_CreateString ===\n");
    
    cJSON *str = cJSON_CreateString("hello world");
    
    if (str != NULL) {
        printf("创建成功！\n");
        printf("type: %d\n", str->type);
        printf("valuestring: %s\n", str->valuestring);
    }
    
    cJSON_Delete(str);
    return 0;
}
#endif

#if 0
// 测试 cJSON_CreateStringReference
#include "cJSON.h"
#include <stdio.h>
int main(void)
{
    printf("=== 测试 cJSON_CreateStringReference ===\n");
    
    const char *external_string = "hello world";
    cJSON *str_ref = cJSON_CreateStringReference(external_string);
    
    if (str_ref != NULL) {
        printf("创建成功!\n");
        printf("type: %d\n", str_ref->type);
        printf("valuestring: %s\n", str_ref->valuestring);
    }
    
    cJSON_Delete(str_ref);
    return 0;
}
#endif

#if 0
// 测试 cJSON_Delete 对于stringreference的处理
#include "cJSON.h"
#include <stdio.h>
int main(void)
{
    printf("=== 测试 cJSON_Delete 处理引用节点 ===\n");
    
    const char *external_string = "hello world";
    cJSON *str_ref = cJSON_CreateStringReference(external_string);
    
    printf("创建成功! type=%d, valuestring=%s\n", 
           str_ref->type, str_ref->valuestring);
    
    // 在这里设置断点调试 cJSON_Delete
    printf("准备删除...\n");
    cJSON_Delete(str_ref);
    printf("删除完成!\n");
    
    return 0;
}
#endif

#if 0
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
// 自定义分配器：带计数
static size_t alloc_count = 0;
static size_t free_count = 0;
static void* my_malloc(size_t size) {
    alloc_count++;
    void* ptr = malloc(size);
    printf("[自定义malloc] size=%zu, ptr=%p, 次数=%zu\n", size, ptr, alloc_count);
    return ptr;
}
static void my_free(void* ptr) {
    free_count++;
    printf("[自定义free] ptr=%p, 次数=%zu\n", ptr, free_count);
    free(ptr);
}
int main(void) {
    printf("=== 测试 cJSON_InitHooks ===\n\n");
    
    // 1. 使用默认分配器
    printf("--- 默认分配器 ---\n");
    cJSON *item1 = cJSON_CreateString("hello");
    printf("创建成功: valuestring=%s\n\n", item1->valuestring);
    cJSON_Delete(item1);
    printf("删除完成\n\n");
    
    // 2. 设置自定义分配器
    printf("--- 设置自定义分配器 ---\n");
    cJSON_Hooks hooks = { my_malloc, my_free };
    cJSON_InitHooks(&hooks);
    
    printf("alloc_count=%zu, free_count=%zu\n\n", alloc_count, free_count);
    
    // 3. 使用自定义分配器创建节点
    printf("--- 使用自定义分配器创建 ---\n");
    cJSON *item2 = cJSON_CreateString("world");
    printf("创建成功: valuestring=%s\n\n", item2->valuestring);
    
    // 4. 删除节点
    printf("--- 删除节点 ---\n");
    cJSON_Delete(item2);
    printf("删除完成\n\n");
    
    printf("统计: alloc=%zu, free=%zu\n", alloc_count, free_count);
    
    // 5. 重置为默认
    printf("\n--- 重置为默认分配器 ---\n");
    cJSON_InitHooks(NULL);
    cJSON *item3 = cJSON_CreateString("default again");
    printf("创建成功: valuestring=%s\n", item3->valuestring);
    cJSON_Delete(item3);
    printf("删除完成\n");
    
    return 0;
}
#endif

#if 0
// 测试 cJSON_Parse - 简单对象 {"a":1}
#include "cJSON.h"
#include <stdio.h>

int main(void) {
    const char *json = "{\"a\":1}";
    cJSON *root = cJSON_Parse(json);
    cJSON_Delete(root);
    return 0;
}
#endif

// 测试 cJSON_Parse - 嵌套对象 {"outer":{"inner":1}}
#include "cJSON.h"
#include <stdio.h>

int main(void) {
    const char *json = "{\"outer\":{\"inner\":1}}";
    cJSON *root = cJSON_Parse(json);
    cJSON_Delete(root);
    return 0;
}