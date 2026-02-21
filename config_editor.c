#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void) {
    // TODO: 读取文件
    FILE *fp = fopen("config.json", "r+");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *json = malloc(file_size + 1);
    if (json == NULL) {
        perror("malloc");
        fclose(fp);
        return 1;
    }
    fread(json, 1, file_size, fp);
    json[file_size] = '\0';

    // TODO: 解析JSON
    cJSON *root = cJSON_Parse(json);
    if (root == NULL) {
        perror("cJSON_Parse");
        fclose(fp);
        free(json);
        return 1;
    }
    // TODO: 查找并修改 port
    cJSON *port_item = cJSON_GetObjectItem(root, "server");
    if (port_item == NULL) {
        perror("cJSON_GetObjectItem");
        cJSON_Delete(root);
        fclose(fp);
        free(json);
        return 1;
    }
    cJSON *port_item_value = cJSON_GetObjectItem(port_item, "port");
    if (port_item_value == NULL) {
        perror("cJSON_GetObjectItem");
        cJSON_Delete(root);
        fclose(fp);
        free(json);
        return 1;
    }
    cJSON_SetIntValue(port_item_value, 9090);

    char *new_json = cJSON_Print(root);
    if (new_json == NULL) {
        perror("cJSON_Print");
        cJSON_Delete(root);
        fclose(fp);
        free(json);
        return 1;
    }

    fclose(fp);
    fp = fopen("config.json", "w");
    if (fp == NULL) {
        perror("fopen for write");
        free(new_json);
        cJSON_Delete(root);
        free(json);
        return 1;
    }
    fwrite(new_json, 1, strlen(new_json), fp);

    free(new_json);
    fclose(fp);
    cJSON_Delete(root);
    free(json);
    return 0;
}