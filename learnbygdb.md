# GDB 调试学习记录 - cJSON

> 目标: 通过 GDB 动态追踪，理解代码执行流程而非静态阅读  
> 学习方法: 提出假设 → 设置断点 → 单步验证 → 修正认知  
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
- **想验证的问题**：...
- **入口点**：函数名 + 行号
- **预期**：我猜代码会怎么做

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

### 会话 1 | 2026-2-15 | 追踪目标：[cJSON_CreateNull]

#### 【调试目标】
- **问题**：我想验证 cJSON_CreateNull 如何创建 null 节点？
- **入口点**：cJSON.c:2622
- **预期路径**：分配内存 → 设置 type = cJSON_NULL → 返回节点

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
- 自动清零是防御性编程，省去手动初始化，但清零逻辑在哪？
- 需要单步进入 cJSON_New_Item 看看具体实现
- cJSON_NULL=4，是 1<<2，位运算定义类型的好处是？

#### 【下一步计划】
- [ ] cJSON_New_Item 内部是如何分配和清零内存的？
- [ ] global_hooks 是如何初始化的？
- [ ] 如果想追踪 cJSON_CreateString，流程是否类似？

#### 【调试截图】

![01](./pics/01.png)

![02](./pics/02.png)

---

## 阶段性总结（可选）

### 已调试的函数
- [x] cJSON_CreateNull (2026-02-15)
- [ ] cJSON_New_Item
- [ ] cJSON_CreateString

### 累积的疑问
- cJSON_New_Item 在哪清零内存？
- 类型标志位的设计意图？
