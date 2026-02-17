# GDB 调试学习记录 - cJSON

> 目标: 通过 GDB 动态追踪，理解代码执行流程而非静态阅读
> 学习方法: 提出假设 -> 设置断点 -> 单步验证 -> 修正认知
> 起始时间: 2026-02-14
> Author: liuyuxin

---

## 调试环境

- **项目路径**: `~/Documents/GitHub/cJSON/`
- **测试文件**: `~/Documents/GitHub/Learn-cJSON-notes/main.c`
- **编译命令**: `gcc -g -o main main.c cJSON.c -lm`
- **GDB 版本**: 9.2

---

## 核心 GDB 命令速查

```gdb
# 断点管理
break <function/line>     # 设置断点
info breakpoints          # 查看所有断点
delete <n>                # 删除第n个断点
disable/enable <n>        # 禁用/启用断点

# 执行控制
run [args]               # 运行程序
step (s)                 # 单步进入函数
next (n)                 # 单步跳过函数
continue (c)             # 继续执行
finish                   # 执行完当前函数

# 变量查看
print <var>              # 打印变量
print/x <var>            # 十六进制显示
print *ptr@n             # 打印数组前n个元素
display <var>            # 每次停下自动显示

# 代码查看
list (l)                 # 显示当前代码
list <func>              # 显示函数代码
backtrace (bt)           # 查看调用栈
frame <n>                # 切换到第n层栈帧
```

---

## 调试会话记录

### 会话结构模板 | YYYY-MM-DD | 追踪目标：[函数名]

#### 【调试目标】
- **想验证的问题**: ...
- **入口点**: 函数名 + 行号
- **预期**: 我猜代码会怎么做

#### 【GDB 命令序列】
```bash
# 编译
gcc -g -o main main.c cJSON.c -lm

# 启动调试
gdb ./main
(gdb) break 函数名
(gdb) run
```

#### 【执行路径记录】
| 步骤 | 位置 | 操作 | 观察结果 |
|------|------|------|----------|
| 1 | 文件:行号 | 操作描述 | 具体观察 |

#### 【变量状态追踪】
```c
// 关键变量的值变化
// (gdb) print var
// 结果
```

#### 【现场想法】
- 临时想到的、还没验证的疑问
- 觉得奇怪的地方
- 下一步想做什么

#### 【截图】
略

---

### 笔记 01 | 2026-2-15 | 追踪目标：[cJSON_CreateNull]

#### 【调试目标】
 - **问题**: 我想验证 cJSON_CreateNull 如何创建 null 节点?
- **入口点**: cJSON.c:2622
- **预期路径**: 分配内存 -> 设置 type = cJSON_NULL -> 返回节点

#### 【GDB 命令序列】
```gdb
# 编译
gcc -g -o main main.c cJSON.c -lm
# 启动调试
gdb ./main
# 设置的断点
(gdb) break cJSON_CreateNull
(gdb) run
```

#### 【执行路径记录】
| 步骤 | 位置 | 操作 | 观察结果 |
|------|------|------|----------|
| 1 | cJSON.c:2623 | 进入函数 | 停在函数入口，尚未执行 |
| 2 | cJSON.c:2624 | 调用 cJSON_New_Item | 完成内存分配，返回堆地址 0x55555555f6b0 |
| 3 | cJSON.c:2625 | 执行 if(item) 判断 | item 非 NULL，条件成立 |
| 4 | cJSON.c:2627 | 执行赋值 item->type = cJSON_NULL | type 从 0 变为 4 |
| 5 | cJSON.c:2630 | 执行 return item | 返回创建的节点 |

#### 【变量状态追踪】
```c
// 步骤 2: 刚分配内存后
(gdb) print item
$3 = (cJSON *) 0x55555555f6b0
(gdb) print *item
$4 = {next = 0x0, prev = 0x0, child = 0x0, type = 0, valuestring = 0x0,
      valueint = 0, valuedouble = 0, string = 0x0}
// 观察: 堆内存地址(0x5555...)，所有字段被初始化为 0

// 步骤 4: 赋值 type 后
(gdb) print item->type
$5 = 4
// 观察: type 被设置为 4，即 cJSON_NULL 的值
```

#### 【关键发现】

**观察到的现象**：
1. 内存分配返回堆地址 `0x55555555f6b0`（以 0x55 开头，是堆内存特征）
2. 分配后所有字段已清零：type=0, next/prev/child=0x0, valuestring=0x0 等
3. 赋值后 type 从 0 变为 4（cJSON_NULL 的值）

#### 【现场想法】
- 自动清零是防御性编程，省去手动初始化，但清零逻辑在哪?
- 需要单步进入 cJSON_New_Item 看看具体实现
- cJSON_NULL=4，是 1<<2，位运算定义类型的好处是?

#### 【下一步计划】
- [x] cJSON_New_Item 内部是如何分配和清零内存的?
- [ ] global_hooks 是如何初始化的?
- [ ] 如果想追踪 cJSON_CreateString，流程是否类似?

#### 【调试截图】

![01](./pics/01.png)

![02](./pics/02.png)

---

### 笔记 02 | 2026-2-16 | 追踪目标：[cJSON_New_Item]

#### 【调试目标】
- **问题**: cJSON_New_Item 如何分配并清零内存?hooks 机制如何工作?
- **入口点**: cJSON.c:294
- **预期路径**: 调用 hooks->allocate -> memset 清零 -> 返回节点

#### 【GDB 命令序列】
```gdb
# 编译
gcc -g -o main main.c cJSON.c -lm
# 启动
gdb ./main
# 设置断点 - 直接在 cJSON_New_Item 入口
(gdb) break cJSON_New_Item
(gdb) run
```

#### 【执行路径记录】
| 步骤 | 位置 | 操作 | 观察结果 |
|------|------|------|----------|
| 1 | cJSON.c:296 | 调用 hooks->allocate | 进入 __GI___libc_malloc，分配 64 字节 |
| 2 | malloc 返回 | finish 返回 | 返回堆地址 0x55555555f6b0 |
| 3 | cJSON.c:297 | 执行 if(node) 判断 | node 非 NULL，条件成立 |
| 4 | cJSON.c:299 | 执行 memset 清零 | 所有字段从垃圾值变为 0 |
| 5 | cJSON.c:302 | 执行 return node | 返回到 cJSON_CreateNull:2624 |
| 6 | cJSON.c:2627 | 赋值 item->type = cJSON_NULL | type 从 0 变为 4 |

#### 【变量状态追踪】
```c
// 步骤 2: malloc 返回值
(gdb) finish
Value returned is $4 = (void *) 0x55555555f6b0
// 观察: 分配 sizeof(cJSON) = 64 字节

// 步骤 2: global_hooks 内容（正确地址在 finish 后显示）
(gdb) print global_hooks
$3 = {allocate = 0x7ffff7e310e0 <__GI___libc_malloc>,
      deallocate = 0x7ffff7e316d0 <__GI___libc_free>,
      reallocate = 0x7ffff7e31e80 <__GI___libc_realloc>}

// 步骤 4: memset 后
(gdb) print *node
$8 = {next = 0x0, prev = 0x0, child = 0x0, type = 0, valuestring = 0x0,
      valueint = 0, valuedouble = 0, string = 0x0}
// 观察: 所有字段被清零

// 步骤 6: type 赋值后
(gdb) print item->type
$10 = 4
(gdb) print *item
$11 = {next = 0x0, prev = 0x0, child = 0x0, type = 4, valuestring = 0x0,
       valueint = 0, valuedouble = 0, string = 0x0}
```

#### 【关键发现】

**观察到的现象**：
1. `global_hooks` 是全局变量，存储 malloc/free/realloc 的 libc 函数指针
2. cJSON_New_Item 通过 `hooks->allocate(sizeof(cJSON))` 间接调用 malloc，分配 64 字节
3. memset 在分配后立即清零，确保所有指针字段为 NULL、数值字段为 0
4. 调用栈：main -> cJSON_CreateNull -> cJSON_New_Item -> malloc

**GDB 显示异常**：
- 断点停在函数入口时，`hooks` 显示为 `0x7ffff7f846a0 <_IO_2_1_stdout_>`（stdout 地址）
- 但 `finish` 返回后，正确显示 `hooks=0x55555555e010 <global_hooks>`
- 这可能是 GDB 对 static 函数参数的调试符号解析问题，但实际执行正确

#### 【现场想法】
- hooks 机制: cJSON 核心代码不直接调用 malloc，而是通过 hooks 间接调用
- 这样用户可以通过 cJSON_InitHooks 替换内存分配器（如嵌入式场景）
- memset 清零是防御性编程，避免使用未初始化的指针导致崩溃
- 64 字节是 cJSON 结构体大小（8 个指针/整数字段）

#### 【已验证的疑问】
- [x] cJSON_New_Item 在哪清零内存? -> cJSON.c:299，memset(node, '\0', sizeof(cJSON))

#### 【下一步计划】
- [ ] cJSON_InitHooks 如何工作?用户如何自定义内存分配器?
- [ ] cJSON_Delete 如何配合 hooks 释放内存?
- [ ] 如果分配失败（malloc 返回 NULL），后续流程如何处理?

#### 【调试截图】

![03](./pics/03.png)

![04](./pics/04.png)

---

### 笔记 03 | 2026-2-16 | 追踪目标：[cJSON_Delete]

#### 【调试目标】
- **问题**: cJSON_Delete 如何释放内存?递归删除如何工作?hooks.deallocate 如何被调用?
- **入口点**: cJSON.c:306
- **预期路径**: 遍历链表 -> 递归删除 child -> 释放 valuestring/string -> 释放自身

#### 【GDB 命令序列】
```gdb
# 编译
gcc -g -o main main.c cJSON.c -lm
# 启动
gdb ./main
# 设置断点
(gdb) break cJSON_Delete
(gdb) run
```

#### 【执行路径记录】
| 步骤 | 位置 | 操作 | 观察结果 |
|------|------|------|----------|
| 1 | cJSON.c:307 | 进入函数 | 停在入口，item = 0x55555555f6b0 |
| 2 | cJSON.c:308 | 定义 next = NULL | next = 0x7fffffffd300（垃圾值，内存中原有的数据） |
| 3 | cJSON.c:309 | while 判断 | item 非 NULL，进入循环 |
| 4 | cJSON.c:311 | next = item->next | next = 0x0（item->next 本身就是 NULL） |
| 5 | cJSON.c:312 | if 判断 child | step 进入，然后 next 跳过 |
| 6 | cJSON.c:319 | if 判断 valuestring | next 跳过 |
| 7 | cJSON.c:324 | if 判断 string | next 跳过 |
| 8 | cJSON.c:329 | deallocate(item) | step 进入 __GI___libc_free |
| 9 | cJSON.c:330 | item = next | item = 0x0 |
| 10 | cJSON.c:309 | while 判断 | item = NULL，退出循环 |

#### 【变量状态追踪】
```c
// 步骤 1: 入口参数
(gdb) print item
$4 = (cJSON *) 0x55555555f6b0
(gdb) print *item
$5 = {next = 0x0, prev = 0x0, child = 0x0, type = 4, valuestring = 0x0,
      valueint = 0, valuedouble = 0, string = 0x0}

// 步骤 2: 定义 next 后（垃圾值）
(gdb) print next
$1 = (cJSON *) 0x7fffffffd300
(gdb) print *next
$2 = {next = 0x1, prev = 0x7fffffffd7bd, child = 0x0, type = -10252, ...}
// 观察: 刚定义的指针，值是内存中原有的垃圾数据

// 步骤 4: 赋值 next = item->next 后
(gdb) print next
$3 = (cJSON *) 0x0
// 观察: item->next = NULL，所以 next = 0

// 步骤 8: 进入 free
(gdb) next
329             global_hooks.deallocate(item);
(gdb) step
__GI___libc_free (mem=0x55555555f6b0) at malloc.c:3087
// 观察: 确认调用的是 free，释放堆地址 0x55555555f6b0

// 步骤 10: 释放后
(gdb) print *item
Cannot access memory at address 0x0
// 观察: 内存已被释放，无法访问
```

#### 【关键发现】

**观察到的现象**：
1. cJSON_Delete 使用 while 循环遍历链表，逐个释放节点
2. 释放顺序：先保存 next -> 递归删除 child -> 释放 valuestring -> 释放 string -> 释放自身
3. 通过 `global_hooks.deallocate` 调用 libc 的 free 函数
4. cJSON_IsReference 标志用于判断 valuestring/child 是否需要释放（引用类型不释放）
5. cJSON_StringIsConst 标志用于判断 string 是否需要释放（常量不释放）
6. 本例中 item 是简单节点（type=4, 无 child, 无 valuestring, 无 string），只释放了自身

**调用栈**：
```
main -> cJSON_Delete -> global_hooks.deallocate -> __GI___libc_free
```

#### 【现场想法】
- 递归删除 child 是为了处理嵌套结构（如数组、对象）
- 引用标志位的设计是为了避免重复释放或释放外部数据
- 释放顺序很重要：先递归释放子节点，再释放自身

#### 【已验证的疑问】
- [x] cJSON_Delete 如何配合 hooks 释放内存? -> cJSON.c:329，global_hooks.deallocate

#### 【下一步计划】
- [x] cJSON_Delete 如何配合 hooks 释放内存? -> cJSON.c:329，global_hooks.deallocate
- [ ] 深入测试：复杂节点（带 child、valuestring）的递归删除过程

#### 【调试截图】

![05](./pics/05.png)

![06](./pics/06.png)

---

### 笔记 04 | 2026-2-16 | 追踪目标：[cJSON_Delete 递归删除]

#### 【调试目标】
- **问题**: 复杂嵌套 JSON 结构的递归删除如何工作?child 指针如何遍历?
- **入口点**: cJSON.c:306
- **测试数据**: `{"name":"test","items":[{"key":"value1"},{"key":"value2"}],"nested":{"inner":"data"}}`

#### 【GDB 命令序列】
```gdb
# 编译
gcc -g -o main main.c cJSON.c -lm
# 启动
gdb ./main
# 设置断点
(gdb) break cJSON_Delete
(gdb) run
```

#### 【执行路径记录】
| 步骤 | 位置 | 操作 | 观察结果 |
|------|------|------|----------|
| 1 | cJSON.c:307 | 进入 root | item = 0x55555555f6b0, type=64(cJSON_Object), child=0x55555555f700 |
| 2 | cJSON.c:315 | 递归调用 cJSON_Delete(child) | 进入 items 数组 (0x55555555f800) |
| 3 | 递归层1 | items 数组有 child | 递归到 items[0] (0x55555555f8e0, object) |
| 4 | 递归层2 | items[0] 有 child | 递归到 {"key":"value1"} (0x55555555f850, string) |
| 5 | 递归层3 | {"key":"value1"} | type=16, valuestring="value1", string="key", 无 child, 释放并返回 |
| 6 | 递归层2 | items[0] 处理完 | next = 0x0, 继续处理 items[1] (0x55555555f930) |
| 7 | 递归层3 | {"key":"value2"} | type=16, valuestring="value2", string="key", 释放并返回 |
| 8 | 递归层2 | items[1] 处理完 | next = 0x0, items 数组遍历完 |
| 9 | 递归层1 | items 处理完 | 移动到 name 节点 (0x55555555f790, type=32, string 乱码) |
| 10 | 递归层1 | name 节点 | 无 valuestring, 直接释放自身 |
| 11 | 递归层1 | name 处理完 | 移动到 nested (0x55555555f9c0, object) |
| 12 | 递归层2 | nested 有 child | 递归到 {"inner":"data"} (0x55555555fa30) |
| 13 | 递归层3 | {"inner":"data"} | type=16, valuestring="data", string="inner", 释放并返回 |
| 14 | 递归层2 | nested 处理完 | 返回递归层1 |
| 15 | 递归层1 | nested 处理完 | 移动到 next = 0x0, 返回 root |
| 16 | root | 处理完所有 child | 释放 root 自身，退出 |

#### 【变量状态追踪】
```c
// 步骤 1: root 入口
(gdb) print *item
$2 = {next = 0x0, prev = 0x0, child = 0x55555555f700, type = 64, valuestring = 0x0, ...}
// type=64 是 cJSON_Object

// 步骤 3: items 数组
(gdb) print *item
$6 = {next = 0x55555555f8e0, prev = 0x55555555f8e0, child = 0x55555555f850, 
      type = 64, ...}
// 这是 items 数组节点，next 指向 items[0]

// 步骤 4: items[0] object
(gdb) print *item
$12 = {next = 0x0, prev = 0x55555555f800, child = 0x55555555f930, 
       type = 64, ...}

// 步骤 5: {"key":"value1"} 叶子节点
(gdb) print *item
$7 = {next = 0x0, prev = 0x55555555f850, child = 0x0, type = 16, 
      valuestring = 0x55555555f8c0 "value1", string = 0x55555555f8a0 "key"}

// 步骤 11: nested object
(gdb) print *item
$19 = {next = 0x0, prev = 0x55555555f790, child = 0x55555555fa30, 
       type = 64, valuestring = 0x0, string = 0x55555555fa10 "nested"}

// 步骤 13: {"inner":"data"} 叶子节点
(gdb) print *item
$20 = {next = 0x0, prev = 0x55555555fa30, child = 0x0, type = 16, 
       valuestring = 0x55555555faa0 "data", string = 0x55555555fa80 "inner"}

// 调用栈变化
// 入口: #0 cJSON_Delete(root) #1 main
// 步骤2: #0 cJSON_Delete(items) #1 cJSON_Delete(root) #2 main
// 步骤4: #0 cJSON_Delete(items[0]) #1 cJSON_Delete(items) #2 cJSON_Delete(root) #3 main
```

#### 【关键发现】

**递归删除机制**：
1. cJSON_Delete 使用 while 循环遍历链表（通过 next 指针）
2. 当遇到有 child 的节点时，递归调用 cJSON_Delete(item->child) 删除子节点
3. 递归是深度优先：先深入到最底层，再逐层返回处理兄弟节点

**JSON 结构与 cJSON 链表对应**：
```
JSON 结构:
{
    "name": "test",
    "items": [{"key":"value1"}, {"key":"value2"}],
    "nested": {"inner":"data"}
}

cJSON 链表结构 (root.child):
items -> name -> nested
         ↓
         items[0] -> items[1]    (items.child)
         ↓
         items[0].child = {"key":"value1"}  (叶子节点)
         ↓
         items[1].child = {"key":"value2"}  (叶子节点)
         ↓
         nested.child = {"inner":"data"}    (叶子节点)
```

**注意**：调试中发现 name 节点的 string 显示为乱码 `"\200\371UUUU"`，这是因为该内存已被释放，GDB 读取到的是垃圾数据。

**释放顺序**：
1. 递归深入到最深层（叶子节点）
2. 释放叶子节点的 valuestring、string、自身
3. 返回上层，释放该节点的子节点
4. 依此类推，直到根节点

**关键代码路径**：
```c
// cJSON.c:312-315
if (!(item->type & cJSON_IsReference) && (item->child != NULL))
{
    cJSON_Delete(item->child);  // 递归调用
}
```

#### 【现场想法】
- 递归删除确保所有嵌套结构都被正确释放
- next 指针用于遍历同一层的兄弟节点
- child 指针用于进入下一层（触发递归）
- 释放 valuestring 和 string 是因为解析时分配了内存

#### 【下一步计划】
- [ ] cJSON_InitHooks 如何工作?
- [ ] cJSON_CreateString 流程

#### 【终端记录】
详见 `scripts/04.txt`
[04.txt](./scripts/04.txt)

---
### 笔记 05 | 2026-2-16 | 追踪目标：[cJSON_CreateString]

#### 【调试目标】
- **问题**: cJSON_CreateString 如何创建字符串节点?cJSON_strdup 做了什么?如何完成的复制?
- **入口点**: cJSON.c:2692
- **预期路径**: cJSON_New_Item -> 设置 type -> cJSON_strdup 复制字符串

#### 【GDB 命令序列】
```bash
# 编译
gcc -g -o main main.c cJSON.c -lm
# 启动
gdb ./main
# 设置的断点
(gdb) break cJSON_CreateString
(gdb) break cJSON_New_Item
(gdb) break cJSON_strdup
(gdb) run
```

#### 【执行路径记录】
| 步骤 | 位置 | 操作 | 观察结果 |
|------|------|------|----------|
| 1 | cJSON.c:2694 | 调用 cJSON_New_Item | 进入 cJSON_New_Item |
| 2 | cJSON.c:296 | 分配内存 | malloc(64) 返回 0x55555555f6b0 |
| 3 | cJSON.c:299 | memset 清零 | 所有字段从垃圾值变为 0 |
| 4 | cJSON.c:302 | return node | 返回节点地址 0x55555555f6b0 |
| 5 | cJSON.c:2697 | 赋值 type | type 从 0 变为 16 (cJSON_String) |
| 6 | cJSON.c:2698 | 调用 cJSON_strdup | 进入 cJSON_strdup |
| 7 | cJSON.c:241 | 计算长度 | length = strlen("hello world") + 1 = 12 |
| 8 | cJSON.c:242 | 分配字符串内存 | hooks->allocate(12) 返回 0x55555555f700 |
| 9 | cJSON.c:247 | 复制字符串 | memcpy(copy, string, 12) |
| 10 | cJSON.c:249 | return copy | 返回复制的字符串地址 |
| 11 | cJSON.c:2698 | 赋值 valuestring | valuestring = 0x55555555f700 |
| 12 | cJSON.c:2706 | return item | 返回创建的节点 |

#### 【变量状态追踪】
```c
// 步骤 4: cJSON_New_Item 返回后
(gdb) print item
$6 = (cJSON *) 0x55555555f6b0
(gdb) print *item
$5 = {next = 0x0, prev = 0x0, child = 0x0, type = 0, valuestring = 0x0, ...}

// 步骤 5: type 赋值后
(gdb) print *item
$8 = {next = 0x0, prev = 0x0, child = 0x0, type = 16, valuestring = 0x0, ...}
// 观察: type = 16 (cJSON_String), valuestring 还是 NULL

// 步骤 9: cJSON_strdup 返回后
(gdb) print copy
$9 = (unsigned char *) 0x55555555f700 "hello world"

// 步骤 11: valuestring 赋值后
(gdb) print *item
$10 = {next = 0x0, prev = 0x0, child = 0x0, type = 16, 
       valuestring = 0x55555555f700 "hello world", valueint = 0, ...}
// 观察: 字符串已复制到新内存，节点持有所有权

// 最终输出
创建成功!
type: 16
valuestring: hello world
```

#### 【关键发现】

**cJSON_CreateString 执行流程**:
1. 调用 cJSON_New_Item 分配 cJSON 节点结构（64 字节）
2. 设置 type = cJSON_String (16)
3. 调用 cJSON_strdup 复制字符串内容到新内存
4. 将复制的字符串地址赋值给 valuestring

**cJSON_strdup 实现机制** (cJSON.c:232-250):
1. 计算字符串长度: strlen(string) + 1（包含结尾 \0）
2. 调用 hooks->allocate 分配内存
3. 使用 memcpy 复制字符串内容
4. 返回新分配的字符串地址

**内存布局**:
```
栈区:                     堆区:
+---------------+         +-------------------+
| "hello world" |         | cJSON 节点 (64B)  |
| (输入参数)     |         | type = 16         |
+---------------+         | valuestring --+---+
                          +---------------|---+
                                          |
                                          v
                                   +-------------------+
                                   | "hello world"     |
                                   | (复制的新内存)      |
                                   +-------------------+
```

**关于 malloc 返回已清零内存的疑问**:
- 观察到 malloc 返回后内存已经被清零（所有字段为 0）
- 这是 glibc malloc 的行为（内存分配器优化）
- cJSON 仍然调用 memset 清零，是防御性编程确保跨平台一致性

#### 【现场想法】
- 字符串节点需要独立分配内存存储字符串内容，与节点结构分离
- cJSON_strdup 实现了类似标准库 strdup 的功能，但使用自定义的内存分配器
- 创建的节点拥有字符串内容的所有权，删除时需要释放两段内存

#### 【下一步计划】
- [ ] cJSON_InitHooks 如何工作?
- [ ] 对比 cJSON_CreateString vs cJSON_CreateStringReference 的区别?

#### 【终端记录】
详见 `scripts/05.txt`
[05.txt](./scripts/05.txt)

---

### 笔记 06 | 2026-2-17 | 追踪目标：[cJSON_CreateStringReference]

#### 【调试目标】
- **问题**: 引用节点与复制节点的区别是什么?cJSON_IsReference 标志如何工作?
- **入口点**: cJSON.c:2709
- **预期路径**: cJSON_New_Item -> 设置 type(含 IsReference) -> 直接引用字符串

#### 【GDB 命令序列】
```bash
# 编译
gcc -g -o main main.c cJSON.c -lm
# 启动
gdb ./main
# 设置断点
(gdb) break cJSON_CreateStringReference
(gdb) break cJSON_New_Item
(gdb) run
```

#### 【执行路径记录】
| 步骤 | 位置 | 操作 | 观察结果 |
|------|------|------|----------|
| 1 | cJSON.c:2711 | 调用 cJSON_New_Item | 分配 64 字节节点 |
| 2 | cJSON.c:296 | 分配内存 | malloc(64) 返回 0x55555555f6b0 |
| 3 | cJSON.c:302 | return node | 返回节点地址 |
| 4 | cJSON.c:2714 | type = cJSON_String \| cJSON_IsReference | type = 272 (16+256) |
| 5 | cJSON.c:2715 | valuestring = cast_away_const(string) | 直接指向外部字符串 |
| 6 | cJSON.c:2718 | return item | 返回节点 |

#### 【变量状态追踪】
```c
// 步骤 3: cJSON_New_Item 返回后
(gdb) print *item
$3 = {next = 0x0, prev = 0x0, child = 0x0, type = 0, valuestring = 0x0, ...}

// 步骤 4: type 赋值后
(gdb) print item->type
$4 = 272

// 步骤 5: valuestring 赋值后
(gdb) print *item
$5 = {next = 0x0, prev = 0x0, child = 0x0, type = 272, 
       valuestring = 0x55555555b033 "hello world", ...}
// 观察: valuestring 直接指向外部字符串地址

// 最终输出
创建成功!
type: 272
valuestring: hello world
```

#### 【关键发现】

**cJSON_CreateStringReference 实现机制**:
1. 分配 cJSON 节点结构（64 字节）
2. 设置 type = cJSON_String | cJSON_IsReference (272)
3. 使用 cast_away_const 将 const char* 转为 char*，直接指向外部字符串
4. 不分配新内存存储字符串内容

**cast_away_const 函数** (cJSON.c:2234):
```c
static void* cast_away_const(const void* string)
{
    return (void*)string;
}
```
- 作用: 去除 const 限定符，使 valuestring 可以指向 const 字符串
- 这是已知安全的类型转换，因为 cJSON 不会修改 valuestring

**内存布局对比**:
```
cJSON_CreateString (复制):
栈区: "hello world"          堆区: [节点] -> [字符串副本]
                                      valuestring 指向堆区

cJSON_CreateStringReference (引用):
栈区: "hello world" --------> 堆区: [节点]
        ↑                      valuestring 指向栈区
    外部字符串地址
```

**两种创建方式对比**:
| 特性 | cJSON_CreateString | cJSON_CreateStringReference |
|------|-------------------|---------------------------|
| 内存分配次数 | 2次 (节点+字符串) | 1次 (仅节点) |
| type 值 | 16 | 272 (16+256) |
| valuestring 指向 | 堆区新分配内存 | 外部字符串地址 |
| 内存所有权 | 节点拥有 | 不拥有，依赖外部生命周期 |
| 删除时释放字符串? | 是 | 否 |

**使用场景**:
- cJSON_CreateString: 普通字符串，cJSON 负责管理内存
- cJSON_CreateStringReference: 常量字符串，避免复制开销，但需外部保证生命周期

#### 【现场想法】
- cJSON_IsReference 标志位通过位运算与 cJSON_String 组合，实现运行时区分
- 引用机制是典型的空间换时间优化，减少内存分配和复制开销
- 使用时需注意: 外部字符串必须比节点存活更久

#### 【下一步计划】
- [x] 对比 cJSON_CreateString vs cJSON_CreateStringReference 的区别?
- [ ] cJSON_Delete 如何处理引用节点?（验证不释放 valuestring）

#### 【调试截图】

![07](./pics/07.png)

![08](./pics/08.png)

#### 【终端记录】
详见 `scripts/06.txt`
[06.txt](./scripts/06.txt)

---

### 笔记 07 | 2026-2-17 | 追踪目标：[cJSON_Delete 处理引用节点]

#### 【调试目标】
- **问题**: cJSON_Delete 如何处理引用节点?是否会释放 valuestring?cJSON_IsReference 标志如何工作?
- **入口点**: cJSON.c:306
- **测试数据**: 使用 cJSON_CreateStringReference 创建的引用节点 (type=272)

#### 【GDB 命令序列】
```bash
# 编译
gcc -g -o main main.c cJSON.c -lm
# 启动
gdb ./main
# 设置断点
(gdb) break cJSON_Delete
(gdb) run
```

#### 【执行路径记录】
| 步骤 | 位置 | 操作 | 观察结果 |
|------|------|------|----------|
| 1 | cJSON.c:307 | 进入函数 | item = 0x7fffffffd210 (初始显示为垃圾值) |
| 2 | cJSON.c:308 | 定义 next = NULL | next = 0x0 |
| 3 | cJSON.c:309 | while 判断 | item 非 NULL，进入循环 |
| 4 | cJSON.c:311 | next = item->next | next = 0x0 |
| 5 | cJSON.c:312 | if 判断 child | !(272 & 256) = 0，条件为 false，跳过递归删除 |
| 6 | cJSON.c:319 | if 判断 valuestring | !(272 & 256) = 0，条件为 false，**跳过释放 valuestring** |
| 7 | cJSON.c:324 | if 判断 string | !(0 & 32) = 1，但 string = NULL，跳过 |
| 8 | cJSON.c:329 | deallocate(item) | 只释放节点自身，不释放 valuestring |
| 9 | cJSON.c:330 | item = next | item = 0x0 |
| 10 | cJSON.c:309 | while 判断 | item = NULL，退出循环 |

#### 【变量状态追踪】
```c
// 步骤 1: 入口参数（初始显示异常）
(gdb) p item
$1 = (cJSON *) 0x7fffffffd210

// 步骤 3: 执行一步后，显示正确
(gdb) print *item
$5 = {next = 0x0, prev = 0x0, child = 0x0, type = 272, valuestring = 0x55555555b037 "hello world", valueint = 0, 
  valuedouble = 0, string = 0x0}
// 观察: type = 272, valuestring 指向栈区外部字符串

// 步骤 5: 检查 cJSON_IsReference 标志
(gdb) p item->type & 256
$7 = 256
(gdb) p !(item->type & cJSON_IsReference)
$8 = 0
// 观察: type & cJSON_IsReference = 256，非零为 true，取反为 false

// 步骤 8: 进入 free
(gdb) step
__GI___libc_free (mem=0x55555555f6b0) at malloc.c:3087
// 观察: 只释放节点自身 (0x55555555f6b0)，valuestring 指向的栈区地址未被释放
```

#### 【关键发现】

**cJSON_Delete 处理引用节点的机制**:
1. 通过 `type & cJSON_IsReference` 判断是否为引用节点
2. 当检测到 cJSON_IsReference 标志时，跳过以下操作:
   - 递归删除 child (第312行)
   - 释放 valuestring (第319行)
3. 只释放节点结构自身，不释放 valuestring 指向的外部内存

**执行流程图**:
```
cJSON_Delete(引用节点):
    while (item != NULL):
        next = item->next
        
        // 关键判断: cJSON_IsReference
        if (!(type & cJSON_IsReference) && child != NULL):
            cJSON_Delete(child)    // 引用节点跳过
        
        if (!(type & cJSON_IsReference) && valuestring != NULL):
            deallocate(valuestring) // 引用节点跳过!!!!!
        
        if (!(type & cJSON_StringIsConst) && string != NULL):
            deallocate(string)
        
        deallocate(item)  // 只释放节点自身
        item = next
```

**内存布局验证**:
```
栈区:                      堆区:
+-------------------+      +-------------------+
| "hello world"     |      | cJSON 节点 (64B)  |
| (外部字符串)       |      | type = 272        |
+-------------------+      | valuestring ------+---> 指向栈区
        ↑                  +-------------------+
        |                          
        +---------------------- 不释放，依赖外部生命周期
```

**输出结果**:
```
=== 测试 cJSON_Delete 处理引用节点 ===
创建成功! type=272, valuestring=hello world
准备删除...
删除完成!
```

#### 【现场想法】
- cJSON_IsReference 设计目的: 避免释放外部管理的内存，防止悬空指针
- 引用节点适用场景: 常量字符串、栈区字符串、已由外部管理的内存
- 释放 valuestring 的条件: 必须同时满足两个条件:
  1. !(type & cJSON_IsReference) - 不是引用节点
  2. valuestring != NULL - 确实分配了内存

#### 【已验证的疑问】
- [x] cJSON_Delete 不释放引用节点的 valuestring? -> 验证成功，第319行跳过

#### 【下一步计划】
- [x] cJSON_Delete 处理引用节点 -> 验证完成
- [ ] cJSON_InitHooks 如何工作?
- [ ] cJSON_Parse 解析流程

#### 【终端记录】
详见 `scripts/07.txt`
[07.txt](./scripts/07.txt)

---

## 阶段性总结（可选）

### 已调试的函数
- [x] cJSON_CreateNull (2026-02-15)
- [x] cJSON_New_Item (2026-02-16)
- [x] cJSON_Delete (2026-02-16)
- [x] cJSON_CreateString (2026-02-16)
- [x] cJSON_CreateStringReference (2026-02-17)
- [ ] cJSON_InitHooks

### 累积的疑问
- 类型标志位的设计意图?
- GDB 显示 hooks 地址异常的原因?
