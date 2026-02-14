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

### 会话 1 | YYYY-MM-DD | 追踪目标：[函数名]

#### 【调试目标】
- **问题**：我想验证什么假设？
- **入口点**：从哪个断点开始？
- **预期路径**：我预计代码会怎么走？

#### 【GDB 命令序列】
```gdb
# 编译
gcc -g -o main main.c cJSON.c -lm

# 启动调试
gdb ./main

# 设置的断点
(gdb) break main
(gdb) break cJSON_Parse
(gdb) run
```

#### 【执行路径记录】
| 步骤 | 位置 | 操作 | 观察结果 |
|------|------|------|----------|
| 1 | main.c:11 | call cJSON_Parse | 传入 JSON 字符串 |
| 2 | cJSON.c:... | ... | ... |

#### 【变量状态追踪】
```c
// 在断点处观察到的关键变量
// (通过 print 命令获取的实际值)
```

#### 【调用栈快照】
```
(gdb) backtrace
#0 ...
#1 ...
```

#### 【认知转折点】

**原先假设**：
- 

**实际观察**：
- 

**认知修正**：
- 

**工程结论**：
- 

#### 【未解问题 / 下一步】
- [ ] 

---

### 会话 2 | YYYY-MM-DD | 追踪目标：[下一个函数]
（同上结构）

---

## 阶段性总结

### 已理解的核心机制
1. 
2. 
3. 

### 仍待探索的问题
- 
- 

### 可迁移的工程经验
- 
