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
    /* 遍历课程数组 */
    cJSON_ArrayForEach(course, courses)
    {
        printf("%s ", cJSON_GetStringValue(course));
    }
    printf("\n");
    
    cJSON_Delete(root);
    
    return 0;
}