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
