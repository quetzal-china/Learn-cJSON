/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__

/* When compiling for windows, we specify a specific calling convention to avoid issues where we are being called from a project with a different default calling convention.  For windows you have 3 define options:

CJSON_HIDE_SYMBOLS - Define this in the case where you don't want to ever dllexport symbols
CJSON_EXPORT_SYMBOLS - Define this on library build when you want to dllexport symbols (default)
CJSON_IMPORT_SYMBOLS - Define this if you want to dllimport symbol

For *nix builds that support visibility attribute, you can define similar behavior by

setting default visibility to hidden by adding
-fvisibility=hidden (for gcc)
or
-xldscope=hidden (for sun cc)
to CFLAGS

then using the CJSON_API_VISIBILITY flag to "export" the same symbols the way CJSON_EXPORT_SYMBOLS does

*/

#define CJSON_CDECL __cdecl
#define CJSON_STDCALL __stdcall

/* export symbols by default, this is necessary for copy pasting the C and header file */
#if !defined(CJSON_HIDE_SYMBOLS) && !defined(CJSON_IMPORT_SYMBOLS) && !defined(CJSON_EXPORT_SYMBOLS)
#define CJSON_EXPORT_SYMBOLS
#endif

#if defined(CJSON_HIDE_SYMBOLS)
#define CJSON_PUBLIC(type)   type CJSON_STDCALL
#elif defined(CJSON_EXPORT_SYMBOLS)
#define CJSON_PUBLIC(type)   __declspec(dllexport) type CJSON_STDCALL
#elif defined(CJSON_IMPORT_SYMBOLS)
#define CJSON_PUBLIC(type)   __declspec(dllimport) type CJSON_STDCALL
#endif
#else /* !__WINDOWS__ */
#define CJSON_CDECL
#define CJSON_STDCALL

#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined (__SUNPRO_C)) && defined(CJSON_API_VISIBILITY)
#define CJSON_PUBLIC(type)   __attribute__((visibility("default"))) type
#else
/* CJSON_PUBLIC(type)实现了把type安全转化 */
#define CJSON_PUBLIC(type) type
#endif
#endif

/* project version */
#define CJSON_VERSION_MAJOR 1
#define CJSON_VERSION_MINOR 7
#define CJSON_VERSION_PATCH 19

#include <stddef.h>

/* cJSON Types: */
/* 组合标签: 使用 | 运算符 */
/* 例如: cJSON_String | cJSON_IsReference 表示一个引用字符串 */
/* 判断标签: 使用 & 运算符 */
/* 例如: item->type & cJSON_String 判断 item 是否为字符串 */
#define cJSON_Invalid (0)
#define cJSON_False  (1 << 0)
#define cJSON_True   (1 << 1)
#define cJSON_NULL   (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array  (1 << 5)
#define cJSON_Object (1 << 6)
#define cJSON_Raw    (1 << 7) /* raw json */

#define cJSON_IsReference 256
#define cJSON_StringIsConst 512

/* =============================================================================
 * cJSON 核心数据结构
 * =============================================================================
 * 设计思想：
 * 1. 统一节点表示：所有 JSON 类型（对象/数组/字符串/数字等）都用同一个结构体表示
 * 2. 双向链表：通过 next/prev 实现同级节点遍历
 * 3. 树形结构：通过 child 指针实现父子关系
 * 4. 类型标志位：通过 type 的位运算组合表示类型和引用状态
 * 
 * 内存布局示例：
 * {"name":"John","age":30}
 * 
 * ┌─────────────────────────────────────────────────────────────┐
 * │                    根节点 (cJSON_Object)                     │
 * │  type: cJSON_Object                                         │
 * │  child ──┐                                                  │
 * └──────────┼──────────────────────────────────────────────────┘
 *            │
 *            ▼
 *     ┌──────────────┐         ┌──────────────┐
 *     │  子节点 1     │ ──next──▶│  子节点 2     │
 *     │  string:"name"│         │  string:"age" │
 *     │  valuestring: │         │  valuedouble: │
 *     │    "John"     │◀─prev── │    30         │
 *     └──────────────┘         └──────────────┘
 * =============================================================================
 */
typedef struct cJSON
{
    /* 双向链表指针：用于遍历同级节点（数组元素或对象键值对） */
    struct cJSON *next;     /* 指向下一个同级节点，最后一个节点为 NULL */
    struct cJSON *prev;     /* 指向上一个同级节点，第一个节点为 NULL */
    
    /* 树形结构指针：指向第一个子节点 */
    struct cJSON *child;    /* 对象/数组类型有效，指向子节点链表的头节点 */

    /* 类型标志位（位运算组合） */
    int type;
    /* 基本类型（低 8 位）:
     *   cJSON_Invalid (0)  - 无效类型
     *   cJSON_False   (1)  - false 布尔值
     *   cJSON_True    (2)  - true 布尔值
     *   cJSON_NULL    (4)  - null 值
     *   cJSON_Number  (8)  - 数字值
     *   cJSON_String  (16) - 字符串值
     *   cJSON_Array   (32) - 数组值
     *   cJSON_Object  (64) - 对象值
     *   cJSON_Raw     (128)- 原始 JSON 字符串
     * 
     * 标志位（高 8 位）:
     *   cJSON_IsReference    (256) - valuestring/child 是引用，不由本节点管理
     *   cJSON_StringIsConst  (512) - string 是常量，不应被释放
     * 
     * 判断方法:
     *   - 判断类型：item->type & cJSON_String
     *   - 判断引用：item->type & cJSON_IsReference
     *   - 组合类型：item->type = cJSON_String | cJSON_IsReference
     */

    /* 值存储字段（根据 type 决定使用哪个字段） */
    char *valuestring;  /* 存储字符串值 (cJSON_String/Raw 类型) */
    int valueint;       /* 存储整数值（已弃用，建议使用 cJSON_SetNumberValue） */
    double valuedouble; /* 存储数字值（cJSON_Number 类型，统一用 double 存储） */

    /* 键名字段（仅当此节点是对象的子节点时有效） */
    char *string;       /* 存储对象的键名，如{"name":"John"}中的"name" */
                        /* 注意：数组元素没有 string 字段（为 NULL） */
} cJSON;

/* =============================================================================
 * 内存管理钩子结构体
 * =============================================================================
 * 设计思想：
 * 1. 集中管理：通过全局 hooks 统一管理所有内存分配，避免 scattered malloc/free
 * 2. 可定制性：允许用户替换为自定义分配器（如内存池、调试分配器）
 * 3. 一致性保证：确保所有 cJSON 内部使用相同的分配/释放函数
 * 
 * 使用场景：
 * - 嵌入式系统：使用静态内存池代替动态分配
 * - 调试模式：使用带泄漏检测的分配器
 * - 性能优化：使用预分配缓冲区
 * =============================================================================
 */
typedef struct cJSON_Hooks
{
      /* Windows 平台必须使用 CDECL 调用约定，确保与标准 malloc/free 兼容 */
      void *(CJSON_CDECL *malloc_fn)(size_t sz);   /* 内存分配函数 */
      void (CJSON_CDECL *free_fn)(void *ptr);      /* 内存释放函数 */
} cJSON_Hooks;

typedef int cJSON_bool;  /* cJSON 自定义布尔类型，1=true, 0=false */

/* 嵌套深度限制：防止恶意 JSON 导致栈溢出 */
/* 原理：parse_object/parse_array 递归调用，每次 depth++，超过限制返回 false */
#ifndef CJSON_NESTING_LIMIT
#define CJSON_NESTING_LIMIT 1000
#endif

/* 循环引用限制：防止环形引用导致无限递归 */
#ifndef CJSON_CIRCULAR_LIMIT
#define CJSON_CIRCULAR_LIMIT 10000
#endif

/* ===========================================================================
 * 版本信息函数
 * ===========================================================================
 * @brief 返回 cJSON 库的版本号字符串
 * @return 格式为 "major.minor.patch" 的静态字符串，如 "1.7.19"
 * 
 * 注意：返回的是静态缓冲区指针，不应被 free 或修改
 * ===========================================================================
 */
CJSON_PUBLIC(const char*) cJSON_Version(void);

/* ===========================================================================
 * 内存管理钩子初始化函数
 * ===========================================================================
 * @brief 设置全局内存分配/释放函数
 * @param hooks 指向 cJSON_Hooks 结构体的指针
 *              - 若为 NULL，则重置为标准 malloc/free
 *              - 若 hooks->malloc_fn 为 NULL，使用默认 malloc
 *              - 若 hooks->free_fn 为 NULL，使用默认 free
 * 
 * 设计思想:
 * 1. 一次性设置：在程序启动时调用一次，全局生效
 * 2. 线程安全警告：多线程环境下必须在任何 cJSON 操作前调用
 * 3. realloc 自动推断：只有当 malloc/free 都是标准库函数时才启用 realloc
 * 
 * 使用示例:
 * // 使用自定义内存分配器
 * cJSON_Hooks hooks = {
 *     .malloc_fn = my_custom_malloc,
 *     .free_fn = my_custom_free
 * };
 * cJSON_InitHooks(&hooks);
 * ===========================================================================
 */
CJSON_PUBLIC(void) cJSON_InitHooks(cJSON_Hooks* hooks);

/* ===========================================================================
 * JSON 解析函数族（4 个变体）
 * ===========================================================================
 * 核心原理：
 * 1. 递归下降解析：parse_value → parse_object/parse_array → 递归调用
 * 2. 错误处理：通过 global_error 记录错误位置
 * 3. 内存分配：使用 global_hooks 统一分配
 * 
 * 返回值：
 * - 成功：指向 cJSON 根节点的指针，调用者负责用 cJSON_Delete 释放
 * - 失败：NULL，可通过 cJSON_GetErrorPtr 获取错误位置
 * ===========================================================================
 */

/* @brief 解析 JSON 字符串（基础版本）
 * @param value 以'\0'结尾的 JSON 字符串
 * @return cJSON 根节点指针，失败返回 NULL
 * 
 * 特点：
 * - 默认不检查末尾垃圾数据
 * - 不返回解析结束位置
 * 
 * 示例：
 * cJSON *json = cJSON_Parse("{\"name\":\"John\"}");
 * if (json == NULL) {
 *     printf("Error at: %s\n", cJSON_GetErrorPtr());
 * }
 */
CJSON_PUBLIC(cJSON *) cJSON_Parse(const char *value);

/* @brief 解析 JSON 字符串（带长度参数）
 * @param value JSON 字符串（可不以'\0'结尾）
 * @param buffer_length 字符串长度
 * @return cJSON 根节点指针，失败返回 NULL
 * 
 * 与 cJSON_Parse 的区别：
 * - 支持非'\0'结尾的字符串
 * - 适合已知长度的缓冲区解析
 */
CJSON_PUBLIC(cJSON *) cJSON_ParseWithLength(const char *value, size_t buffer_length);

/* @brief 解析 JSON 字符串（带选项控制）
 * @param value 要解析的 JSON 字符串
 * @param return_parse_end 输出参数，接收解析结束位置（可为 NULL）
 *                         - 成功：指向 JSON 后的第一个字符
 *                         - 失败：指向错误发生位置
 * @param require_null_terminated 是否要求 JSON 后必须是 '\0'
 *                                  - 1：严格模式，JSON 后有垃圾数据则失败
 *                                  - 0：宽松模式，忽略后续数据
 * @return cJSON 根节点指针，失败返回 NULL
 * 
 * 优势：
 * - 线程安全：通过 return_parse_end 代替 cJSON_GetErrorPtr
 * - 精确控制：可解析连续存储的多个 JSON
 * 
 * 示例：
 * const char *json_text = "{\"key\":\"value\"} extra garbage";
 * const char *end_ptr;
 * cJSON *json = cJSON_ParseWithOpts(json_text, &end_ptr, 0);
 * printf("Parsed %ld bytes\n", end_ptr - json_text);
 */
CJSON_PUBLIC(cJSON *) cJSON_ParseWithOpts(const char *value, const char **return_parse_end, cJSON_bool require_null_terminated);

/* @brief 解析 JSON 字符串（完整选项 + 长度控制）
 * @param value JSON 字符串
 * @param buffer_length 字符串长度
 * @param return_parse_end 输出参数，解析结束位置
 * @param require_null_terminated 是否要求'\0'结尾
 * @return cJSON 根节点指针，失败返回 NULL
 * 
 * 最完整的解析接口，其他函数都是此函数的包装
 */
CJSON_PUBLIC(cJSON *) cJSON_ParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, cJSON_bool require_null_terminated);

/* ===========================================================================
 * JSON 打印/序列化函数族
 * ===========================================================================
 * 核心原理:
 * 1. 递归序列化:print_value → print_object/print_array → 递归调用
 * 2. 动态缓冲区:使用 printbuffer 结构体管理输出缓冲区，自动扩容
 * 3. 格式化控制:支持格式化（带缩进换行）和紧凑输出两种模式
 * 
 * 内存管理:
 * - 返回的字符串由 malloc 分配，调用者必须用 free 释放
 * - 失败返回 NULL
 * ===========================================================================
 */

/* @brief 序列化 cJSON 为格式化 JSON 字符串
 * @param item 要序列化的 cJSON 根节点
 * @return 格式化后的 JSON 字符串（带缩进和换行），失败返回 NULL
 * 
 * 输出示例:
 * {
 *     "name": "John",
 *     "age": 30
 * }
 * 
 * 注意：返回的字符串必须用 free 释放
 */
CJSON_PUBLIC(char *) cJSON_Print(const cJSON *item);

/* @brief 序列化 cJSON 为紧凑 JSON 字符串
 * @param item 要序列化的 cJSON 根节点
 * @return 紧凑格式的 JSON 字符串（无空白字符），失败返回 NULL
 * 
 * 输出示例：
 * {"name":"John","age":30}
 * 
 * 适用场景：
 * - 网络传输（减少带宽）
 * - 文件存储（减少空间）
 */
CJSON_PUBLIC(char *) cJSON_PrintUnformatted(const cJSON *item);

/* @brief 使用预分配缓冲区策略序列化 JSON
 * @param item 要序列化的 cJSON 根节点
 * @param prebuffer 初始缓冲区大小（猜测值，合理猜测可减少 realloc）
 * @param fmt 是否格式化输出（1=格式化，0=紧凑）
 * @return JSON 字符串，失败返回 NULL
 * 
 * 优化原理:
 * - 预先分配 prebuffer 字节，避免频繁 realloc
 * - 适合对性能敏感的场景
 */
CJSON_PUBLIC(char *) cJSON_PrintBuffered(const cJSON *item, int prebuffer, cJSON_bool fmt);

/* @brief 使用外部提供的缓冲区序列化 JSON
 * @param item 要序列化的 cJSON 根节点
 * @param buffer 外部提供的缓冲区（必须有足够空间）
 * @param length 缓冲区长度
 * @param format 是否格式化输出
 * @return 1=成功，0=失败（缓冲区太小）
 * 
 * 适用场景:
 * - 嵌入式系统（无动态分配）
 * - 实时系统（不允许 malloc）
 * 
 * 注意:
 * - 失败时不会释放任何内存
 * - 调用者完全负责缓冲区管理
 */
CJSON_PUBLIC(cJSON_bool) cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const cJSON_bool format);
/* ===========================================================================
 * 删除/释放函数
 * ===========================================================================
 * 核心原理:
 * 1. 后序遍历：先递归删除所有子节点，再删除自身
 * 2. 引用检查：若节点标记为引用（cJSON_IsReference），则不释放其 child/valuestring
 * 3. 迭代删除：通过 next 指针遍历兄弟节点链表
 * ===========================================================================
 */

/* @brief 递归删除 cJSON 节点及其所有子节点
 * @param item 要删除的 cJSON 节点
 * 
 * 删除逻辑：
 * 1. 若 item->type & cJSON_IsReference: 不删除 child 和 valuestring
 * 2. 若 item->type & cJSON_StringIsConst: 不删除 string
 * 3. 否则：释放所有分配的内存
 * 
 * 示例:
 * cJSON *json = cJSON_Parse("{\"a\":1}");
 * // 使用 json...
 * cJSON_Delete(json);  // 必须调用，否则内存泄漏
 */
CJSON_PUBLIC(void) cJSON_Delete(cJSON *item);

/* ===========================================================================
 * 数组/对象访问函数
 * ===========================================================================
 * 性能警告:
 * - 链表结构导致 O(n) 查找时间
 * - 避免在循环中使用 cJSON_GetArrayItem（应用 cJSON_ArrayForEach 宏）
 * ===========================================================================
 */

/* @brief 获取数组大小（元素个数）
 * @param array 数组类型的 cJSON 节点
 * @return 元素个数，参数为 NULL 时返回 0
 * 
 * 实现原理：遍历 next 链表计数
 * 时间复杂度：O(n)
 */
CJSON_PUBLIC(int) cJSON_GetArraySize(const cJSON *array);

/* @brief 获取数组指定索引的元素
 * @param array 数组类型的 cJSON 节点
 * @param index 索引（从 0 开始）
 * @return 指向索引元素的指针，失败返回 NULL
 * 
 * 时间复杂度：O(n)
 * 注意：index < 0 时返回 NULL
 */
CJSON_PUBLIC(cJSON *) cJSON_GetArrayItem(const cJSON *array, int index);

/* @brief 获取对象的键值（不区分大小写）
 * @param object 对象类型的 cJSON 节点
 * @param string 要查找的键名
 * @return 匹配的键值节点，失败返回 NULL
 * 
 * 注意：
 * - 不区分大小写（"Name" == "name"）
 * - 若有重复键名，返回第一个匹配项
 */
CJSON_PUBLIC(cJSON *) cJSON_GetObjectItem(const cJSON * const object, const char * const string);

/* @brief 获取对象的键值（区分大小写）
 * @param object 对象类型的 cJSON 节点
 * @param string 要查找的键名
 * @return 匹配的键值节点，失败返回 NULL
 * 
 * 注意：
 * - 区分大小写（"Name" != "name"）
 * - 符合 JSON 标准的行为
 */
CJSON_PUBLIC(cJSON *) cJSON_GetObjectItemCaseSensitive(const cJSON * const object, const char * const string);

/* @brief 检查对象是否包含某键
 * @param object 对象类型的 cJSON 节点
 * @param string 要检查的键名
 * @return 1=包含，0=不包含
 */
CJSON_PUBLIC(cJSON_bool) cJSON_HasObjectItem(const cJSON *object, const char *string);

/* @brief 获取解析错误位置
 * @return 指向错误位置的字符串指针
 * 
 * 警告：线程不安全！
 * - 多线程环境下应使用 cJSON_ParseWithOpts + return_parse_end
 */
CJSON_PUBLIC(const char *) cJSON_GetErrorPtr(void);

/* ===========================================================================
 * 值获取函数
 * ===========================================================================
 * 设计理念：
 * 1. 类型安全：先检查类型，再返回值
 * 2. 空指针友好：参数为 NULL 或类型不匹配时返回安全的默认值
 * ===========================================================================
 */

/* @brief 获取字符串值
 * @param item cJSON 节点
 * @return 字符串值，类型不匹配或 NULL 时返回 NULL
 * 
 * 示例:
 * cJSON *json = cJSON_Parse("{\"name\":\"John\"}");
 * const char *name = cJSON_GetStringValue(cJSON_GetObjectItem(json, "name"));
 * // name = "John"
 */
CJSON_PUBLIC(char *) cJSON_GetStringValue(const cJSON * const item);

/* @brief 获取数字值
 * @param item cJSON 节点
 * @return 数值，类型不匹配或 NULL 时返回 NaN
 * 
 * 注意：返回 double 类型，可表示整数和浮点数
 */
CJSON_PUBLIC(double) cJSON_GetNumberValue(const cJSON * const item);

/* ===========================================================================
 * 类型检查函数（返回 cJSON_bool）
 * ===========================================================================
 * 实现原理：直接比较 type 字段
 * 线程安全：纯函数，无副作用
 * ===========================================================================
 */
CJSON_PUBLIC(cJSON_bool) cJSON_IsInvalid(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsFalse(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsTrue(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsBool(const cJSON * const item);   /* true 或 false */
CJSON_PUBLIC(cJSON_bool) cJSON_IsNull(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsNumber(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsString(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsArray(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsObject(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsRaw(const cJSON * const item);

/* ===========================================================================
 * 创建节点函数族
 * ===========================================================================
 * 核心原理:
 * 1. 统一分配：都通过 cJSON_New_Item 分配节点
 * 2. 类型设置：分配后立即设置 type 字段
 * 3. 内存责任：返回的节点必须由调用者用 cJSON_Delete 释放
 *              （除非已添加到其他节点，所有权转移）
 * 
 * 设计思想:
 * - 零值初始化：新节点所有字段初始化为 0/NULL
 * - 最小分配：只分配 cJSON 结构体，不预分配子节点空间
 * ===========================================================================
 */

/* @brief 创建 null 节点
 * @return 新创建的 cJSON 节点，type=cJSON_NULL
 * 
 * JSON 表示：null
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateNull(void);

/* @brief 创建 true 节点
 * @return 新创建的 cJSON 节点，type=cJSON_True
 * 
 * JSON 表示：true
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateTrue(void);

/* @brief 创建 false 节点
 * @return 新创建的 cJSON 节点，type=cJSON_False
 * 
 * JSON 表示：false
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateFalse(void);

/* @brief 创建布尔节点
 * @param boolean 布尔值（1=true, 0=false）
 * @return 新创建的 cJSON 节点，type=cJSON_True 或 cJSON_False
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateBool(cJSON_bool boolean);

/* @brief 创建数字节点
 * @param num 数值（double 类型）
 * @return 新创建的 cJSON 节点，type=cJSON_Number
 * 
 * 内部存储：
 * - valuedouble = num（完整精度）
 * - valueint = (int)num（截断，溢出时饱和处理）
 * 
 * JSON 表示：42, 3.14, -1.5e10
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateNumber(double num);

/* @brief 创建字符串节点（复制字符串）
 * @param string 要复制的字符串（以'\0'结尾）
 * @return 新创建的 cJSON 节点，type=cJSON_String
 * 
 * 内存管理:
 * - 内部会 strdup 复制字符串
 * - 删除节点时会自动释放
 * 
 * JSON 表示："hello"
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateString(const char *string);

/* @brief 创建字符串节点（引用外部字符串）
 * @param string 外部字符串指针（必须长期有效）
 * @return 新创建的 cJSON 节点，type=cJSON_String | cJSON_IsReference
 * 
 * 与普通字符串的区别:
 * - 不复制字符串，直接指向外部内存
 * - 删除节点时不会释放 valuestring
 * - 适合常量字符串或生命周期更长的缓冲区
 * 
 * 警告：
 * - 若外部字符串被释放，会产生悬空指针
 * - 不可修改 valuestring
 * 
 * 示例:
 * const char *static_str = "static";
 * cJSON *item = cJSON_CreateStringReference(static_str);
 * // 不需要管理 static_str 的生命周期
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateStringReference(const char *string);

/* @brief 创建原始 JSON 节点
 * @param raw 原始 JSON 字符串
 * @return 新创建的 cJSON 节点，type=cJSON_Raw
 * 
 * 适用场景:
 * - 直接嵌入预格式化的 JSON
 * - 避免重复序列化
 * 
 * 注意：不会验证 raw 是否为有效 JSON
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateRaw(const char *raw);

/* @brief 创建空数组节点
 * @return 新创建的 cJSON 节点，type=cJSON_Array
 * 
 * JSON 表示：[]
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateArray(void);

/* @brief 创建空对象节点
 * @return 新创建的 cJSON 节点，type=cJSON_Object
 * 
 * JSON 表示：{}
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateObject(void);

/* @brief 创建数组引用节点
 * @param child 被引用的数组节点
 * @return 新创建的 cJSON 节点，type=cJSON_Array | cJSON_IsReference
 * 
 * 特点：
 * - 不拥有 child，删除时不释放
 * - 适合临时引用
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateArrayReference(const cJSON *child);

/* @brief 创建对象引用节点
 * @param child 被引用的对象节点
 * @return 新创建的 cJSON 节点，type=cJSON_Object | cJSON_IsReference
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateObjectReference(const cJSON *child);

/* ===========================================================================
 * 批量创建数组函数族
 * ===========================================================================
 * 功能：一次性从 C 数组创建 cJSON 数组
 * 参数:
 * - numbers/strings: C 语言数组指针
 * - count: 数组元素个数
 * @return cJSON 数组节点
 * ===========================================================================
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateIntArray(const int *numbers, int count);
CJSON_PUBLIC(cJSON *) cJSON_CreateFloatArray(const float *numbers, int count);
CJSON_PUBLIC(cJSON *) cJSON_CreateDoubleArray(const double *numbers, int count);
CJSON_PUBLIC(cJSON *) cJSON_CreateStringArray(const char *const *strings, int count);

/* ===========================================================================
 * 添加/插入节点函数
 * ===========================================================================
 * 所有权规则:
 * 1. 添加后，item 的所有权转移给 array/object
 * 2. 不应再用 cJSON_Delete 删除已添加的 item
 * 3. 删除父节点时会自动删除所有子节点
 * 
 * 链表操作:
 * - 新节点总是添加到链表末尾
 * - 通过 prev 指针优化，O(1) 时间找到末尾
 * ===========================================================================
 */

/* @brief 添加节点到数组末尾
 * @param array 数组节点
 * @param item 要添加的节点
 * @return 1=成功，0=失败（参数为 NULL）
 * 
 * 示例:
 * cJSON *arr = cJSON_CreateArray();
 * cJSON_AddItemToArray(arr, cJSON_CreateNumber(42));
 */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToArray(cJSON *array, cJSON *item);

/* @brief 添加键值对到对象
 * @param object 对象节点
 * @param string 键名（会被 strdup 复制）
 * @param item 要添加的值节点
 * @return 1=成功，0=失败
 * 
 * 示例:
 * cJSON *obj = cJSON_CreateObject();
 * cJSON_AddItemToObject(obj, "name", cJSON_CreateString("John"));
 */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item);

/* @brief 添加键值对到对象（键名为常量字符串）
 * @param object 对象节点
 * @param string 键名（必须是常量或长期有效的字符串）
 * @param item 要添加的值节点
 * @return 1=成功，0=失败
 * 
 * 与普通版本的区别:
 * - 不复制 string，直接引用
 * - 节省内存，但要求 string 长期有效
 * 
 * 警告：
 * - string 必须设置 cJSON_StringIsConst 标志
 * - 不可修改或释放 string
 */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToObjectCS(cJSON *object, const char *string, cJSON *item);

/* @brief 添加引用到数组
 * @param array 数组节点
 * @param item 被引用的节点
 * @return 1=成功，0=失败
 * 
 * 特点:
 * - 创建 item 的引用副本（cJSON_IsReference）
 * - 原 item 不受影响，可继续使用
 */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item);

/* @brief 添加引用到对象
 * @param object 对象节点
 * @param string 键名
 * @param item 被引用的节点
 * @return 1=成功，0=失败
 */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemReferenceToObject(cJSON *object, const char *string, cJSON *item);

/* ===========================================================================
 * 删除/分离节点函数
 * ===========================================================================
 * Detach vs Delete:
 * - Detach: 从父节点移除，但返回节点指针（调用者负责释放）
 * - Delete: 从父节点移除并直接释放
 * ===========================================================================
 */

/* @brief 通过指针分离节点
 * @param parent 父节点（数组或对象）
 * @param item 要分离的子节点
 * @return 分离后的节点，失败返回 NULL
 * 
 * 适用场景：
 * - 需要将子节点移动到另一个父节点
 */
CJSON_PUBLIC(cJSON *) cJSON_DetachItemViaPointer(cJSON *parent, cJSON * const item);

/* @brief 从数组分离指定索引的元素
 * @param array 数组节点
 * @param which 元素索引
 * @return 分离后的节点
 */
CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromArray(cJSON *array, int which);

/* @brief 从数组删除指定索引的元素
 * @param array 数组节点
 * @param which 元素索引
 * 
 * 实现：Detach + Delete
 */
CJSON_PUBLIC(void) cJSON_DeleteItemFromArray(cJSON *array, int which);

/* @brief 从对象分离指定键的键值（不区分大小写）
 * @param object 对象节点
 * @param string 键名
 * @return 分离后的节点
 */
CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromObject(cJSON *object, const char *string);

/* @brief 从对象分离指定键的键值（区分大小写）
 * @param object 对象节点
 * @param string 键名
 * @return 分离后的节点
 */
CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromObjectCaseSensitive(cJSON *object, const char *string);

/* @brief 从对象删除指定键的键值（不区分大小写）
 */
CJSON_PUBLIC(void) cJSON_DeleteItemFromObject(cJSON *object, const char *string);

/* @brief 从对象删除指定键的键值（区分大小写）
 */
CJSON_PUBLIC(void) cJSON_DeleteItemFromObjectCaseSensitive(cJSON *object, const char *string);

/* ===========================================================================
 * 更新/替换节点函数
 * ===========================================================================
 */

/* @brief 在数组指定位置插入新节点
 * @param array 数组节点
 * @param which 插入位置（0 开始）
 * @param newitem 新节点
 * @return 1=成功，0=失败
 * 
 * 效果：
 * - 原位置及之后的元素向右移动
 */
CJSON_PUBLIC(cJSON_bool) cJSON_InsertItemInArray(cJSON *array, int which, cJSON *newitem);

/* @brief 通过指针替换节点
 * @param parent 父节点
 * @param item 被替换的旧节点
 * @param replacement 新节点
 * @return 1=成功，0=失败
 * 
 * 实现:
 * 1.  detach 旧节点
 * 2.  delete 旧节点
 * 3.  insert 新节点
 */
CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemViaPointer(cJSON * const parent, cJSON * const item, cJSON * replacement);

/* @brief 替换数组指定位置的元素
 */
CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInArray(cJSON *array, int which, cJSON *newitem);

/* @brief 替换对象指定键的值（不区分大小写）
 */
CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInObject(cJSON *object,const char *string,cJSON *newitem);

/* @brief 替换对象指定键的值（区分大小写）
 */
CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInObjectCaseSensitive(cJSON *object,const char *string,cJSON *newitem);

/* ===========================================================================
 * 复制与比较函数
 * ===========================================================================
 */

/* @brief 复制 cJSON 节点
 * @param item 要复制的源节点
 * @param recurse 是否递归复制子节点
 *                - 1: 深拷贝，复制整个树
 *                - 0: 浅拷贝，只复制根节点
 * @return 新复制的节点
 * 
 * 注意:
 * - 返回的节点必须用 cJSON_Delete 释放
 * - 复制后节点的 next/prev 指针为 NULL
 */
CJSON_PUBLIC(cJSON *) cJSON_Duplicate(const cJSON *item, cJSON_bool recurse);

/* @brief 比较两个 cJSON 节点是否相等
 * @param a 第一个节点
 * @param b 第二个节点
 * @param case_sensitive 对象键名是否区分大小写
 * @return 1=相等，0=不相等
 * 
 * 比较规则:
 * - 类型必须相同
 * - 数值比较用 double 精度
 * - 字符串比较用 strcmp
 * - 数组/对象递归比较所有元素
 */
CJSON_PUBLIC(cJSON_bool) cJSON_Compare(const cJSON * const a, const cJSON * const b, const cJSON_bool case_sensitive);

/* ===========================================================================
 * 工具函数
 * ===========================================================================
 */

/* @brief 移除 JSON 字符串中的空白字符
 * @param json 可读写的 JSON 字符串（不能是字符串常量！）
 * 
 * 移除的字符:
 * - 空格 ' '
 * - 制表符 '\t'
 * - 回车符 '\r'
 * - 换行符 '\n'
 * 
 * 警告：
 * - 会修改输入字符串
 * - 不可用于字符串常量
 */
CJSON_PUBLIC(void) cJSON_Minify(char *json);

/* ===========================================================================
 * 辅助函数（创建并添加到对象）
 * ===========================================================================
 * 设计理念：
 * 1. 原子操作：创建和添加在一个函数内完成
 * 2. 错误处理：失败返回 NULL，方便链式调用检查
 * 3. 简化代码：减少临时变量声明
 * 
 * 使用模式:
 * if (cJSON_AddStringToObject(obj, "name", "John") == NULL) {
 *     // 处理内存分配失败
 * }
 * ===========================================================================
 */
CJSON_PUBLIC(cJSON*) cJSON_AddNullToObject(cJSON * const object, const char * const name);
CJSON_PUBLIC(cJSON*) cJSON_AddTrueToObject(cJSON * const object, const char * const name);
CJSON_PUBLIC(cJSON*) cJSON_AddFalseToObject(cJSON * const object, const char * const name);
CJSON_PUBLIC(cJSON*) cJSON_AddBoolToObject(cJSON * const object, const char * const name, const cJSON_bool boolean);
CJSON_PUBLIC(cJSON*) cJSON_AddNumberToObject(cJSON * const object, const char * const name, const double number);
CJSON_PUBLIC(cJSON*) cJSON_AddStringToObject(cJSON * const object, const char * const name, const char * const string);
CJSON_PUBLIC(cJSON*) cJSON_AddRawToObject(cJSON * const object, const char * const name, const char * const raw);
CJSON_PUBLIC(cJSON*) cJSON_AddObjectToObject(cJSON * const object, const char * const name);
CJSON_PUBLIC(cJSON*) cJSON_AddArrayToObject(cJSON * const object, const char * const name);

/* ===========================================================================
 * 数值设置宏与辅助函数
 * ===========================================================================
 * 设计原因:
 * - JSON 标准不区分整数和浮点数
 * - cJSON 内部统一用 double 存储
 * - valueint 仅为兼容保留
 * 
 * 饱和处理:
 * - number >= INT_MAX → valueint = INT_MAX
 * - number <= INT_MIN → valueint = INT_MIN
 * ===========================================================================
 */

/* @brief 设置整数值（宏）
 * @param object cJSON 节点
 * @param number 要设置的整数值
 * @return 设置的值
 * 
 * 效果：同时设置 valueint 和 valuedouble
 */
#define cJSON_SetIntValue(object, number) ((object) ? (object)->valueint = (object)->valuedouble = (number) : (number))

/* @brief 设置数值辅助函数（内部使用）
 * @param object cJSON 节点
 * @param number 要设置的数值
 * @return 设置的值
 */
CJSON_PUBLIC(double) cJSON_SetNumberHelper(cJSON *object, double number);

/* @brief 设置数值（推荐使用的宏）
 * @param object cJSON 节点
 * @param number 要设置的数值
 * @return 设置的值
 * 
 * 使用示例:
 * cJSON *num = cJSON_CreateNumber(0);
 * cJSON_SetNumberValue(num, 3.14);
 */
#define cJSON_SetNumberValue(object, number) ((object != NULL) ? cJSON_SetNumberHelper(object, (double)number) : (number))

/* ===========================================================================
 * 字符串值设置函数
 * ===========================================================================
 * @brief 修改字符串节点的 valuestring
 * @param object 字符串类型的 cJSON 节点
 * @param valuestring 新的字符串值
 * @return 新的 valuestring 指针，失败返回 NULL
 * 
 * 限制:
 * - 仅当 type == cJSON_String 且非引用类型时有效
 * - 引用节点 (cJSON_IsReference) 不可修改
 * 
 * 内存管理:
 * - 若新字符串更长，会重新分配内存
 * - 旧字符串会被释放
 */
CJSON_PUBLIC(char*) cJSON_SetValuestring(cJSON *object, const char *valuestring);

/* ===========================================================================
 * 布尔值设置宏
 * ===========================================================================
 * @brief 设置布尔节点的布尔值
 * @param object 布尔类型的 cJSON 节点
 * @param boolValue 新的布尔值（1=true, 0=false）
 * @return 新的类型值，失败返回 cJSON_Invalid
 * 
 * 限制:
 * - 仅当节点类型为 cJSON_True 或 cJSON_False 时有效
 */
#define cJSON_SetBoolValue(object, boolValue) ( \
    (object != NULL && ((object)->type & (cJSON_False|cJSON_True))) ? \
    (object)->type=((object)->type &(~(cJSON_False|cJSON_True)))|((boolValue)?cJSON_True:cJSON_False) : \
    cJSON_Invalid\
)

/* ===========================================================================
 * 数组/对象遍历宏
 * ===========================================================================
 * @brief 遍历数组或对象的便捷宏
 * @param element 循环变量（cJSON* 类型，需预先声明）
 * @param array 要遍历的数组或对象
 * 
 * 实现原理:
 * - 展开为 for 循环
 * - 通过 child 获取第一个元素
 * - 通过 next 遍历后续元素
 * 
 * 使用示例:
 * cJSON *element;
 * cJSON_ArrayForEach(element, array) {
 *     // 处理 element
 * }
 * 
 * 优势:
 * - O(n) 时间复杂度（相比索引访问的 O(n²)）
 * - 代码简洁
 */
#define cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

/* ===========================================================================
 * 内存管理函数
 * ===========================================================================
 * @brief 使用 cJSON 的内存分配器分配内存
 * @param size 要分配的字节数
 * @return 分配的内存指针，失败返回 NULL
 * 
 * 用途:
 * - 与 cJSON 内部使用相同的分配器
 * - 确保内存一致性
 */
CJSON_PUBLIC(void *) cJSON_malloc(size_t size);

/* @brief 使用 cJSON 的内存释放器释放内存
 * @param object 要释放的内存指针
 * 
 * 用途:
 * - 释放 cJSON_malloc 分配的内存
 * - 释放 cJSON_Print 返回的字符串
 */
CJSON_PUBLIC(void) cJSON_free(void *object);

#ifdef __cplusplus
}
#endif

#endif
