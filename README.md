# cJSON 源码学习项目

> NPU软件学院2025级学院任务1
> 项目名称：深入理解 cJSON 开源项目源码  
> 学习方法：静态代码分析 + 动态调试追踪 + 认知笔记记录

---

## 项目概述

本项目是对 [cJSON](https://github.com/DaveGamble/cJSON) 开源库的系统性学习研究。

**学习目标**：
- 理解递归下降解析器的工作原理
- 掌握 C 语言内存管理技巧
- 学习工程化的代码组织方式
- 培养阅读大型开源项目的能力

---

## 文件结构说明

### 源码文件（Source Code）

| 文件名 | 说明 | 来源 |
|--------|------|------|
| `cJSON.c` | cJSON 核心实现源码（约3000行） | 官方仓库 v1.7.19 |
| `cJSON.h` | cJSON 头文件，包含 API 声明 | 官方仓库 v1.7.19 |

**学习重点**：
- JSON 解析：`cJSON_Parse()` → `parse_value()` → `parse_object()` / `parse_array()`
- JSON 生成：`cJSON_CreateObject()`, `cJSON_AddItemToObject()`
- 内存管理：`cJSON_InitHooks()` 全局内存分配器机制
- 类型系统：`cJSON_IsReference` 等标志位设计

### 测试代码（Test Code）

| 文件名 | 说明 | 用途 |
|--------|------|------|
| `main.c` | **自主编写的测试程序** | 验证学习成果，演示 cJSON 基本用法 |
| `test.c` | 官方测试代码 | 参考学习，了解单元测试编写方式 |

**`main.c` 功能演示**：
```c
// 解析复杂 JSON 字符串
{"name":"John","age":30,"is_student":false,"courses":["math","english"]}

// 展示的能力：
// 1. JSON 对象解析
// 2. 数组遍历（cJSON_ArrayForEach）
// 3. 类型判断与取值
```

### 学习笔记（Learning Notes）

#### 1. `thinking.md` - 认知转折点记录
**记录方法**：按时间顺序记录学习过程中的关键认知突破

**已记录内容**：
- **2026-02-09** | `internal_hooks` 内存分配机制
  - 发现：cJSON 通过全局 hooks 统一管理内存分配
  - 价值：学习到"集中管理策略"的工程思想
  
- **2026-02-10** | `strtod` 类型转换问题
  - 发现：`unsigned char**` 到 `char**` 的转换基于约定而非类型安全
  - 价值：理解 C 语言中语义安全与类型系统的关系

- **2026-02-11** | `cJSON_SetValuestring` 引用节点处理
  - 发现：`cJSON_IsReference` 标志位用于区分所有权模式
  - 价值：学习到标志位在运行时决策中的应用

#### 2. `learnbygdb.md` - GDB 调试学习记录
**记录方法**：通过动态调试追踪代码执行流程

**内容结构**：
- GDB 常用命令速查表
- 调试会话记录（断点 → 执行路径 → 变量观察 → 认知修正）
- 阶段性总结

**学习方法**：
```
提出假设 → 设置断点 → 单步执行 → 验证/修正假设 → 记录结论
```

#### 3. `CJSON_ARCHITECTURE.md` - 架构全景分析
**内容**：
- 核心数据结构图解（cJSON 节点结构）
- 内存管理架构图
- JSON 解析流程图
- JSON 打印流程图
- 工程构建思路（从零到有的设计过程）
- 关键设计决策分析

### 参考文档（Reference）

| 文件名 | 说明 |
|--------|------|
| `MainREADME.md` | cJSON 官方 README（学习参考） |

---

## 学习方法总结

### 阶段一：建立地图（第1周）
1. 阅读官方 README，了解项目功能
2. 浏览文件结构，识别核心模块
3. 编译运行示例代码

### 阶段二：静态分析（第2-3周）
1. 阅读 `cJSON.h`，理解数据结构定义
2. 选择核心函数（`cJSON_Parse`）逐行阅读
3. 记录不理解的地方，形成问题清单

### 阶段三：动态调试（第4周起）
1. 使用 GDB 设置断点，单步跟踪
2. 观察变量状态变化，理解执行流程
3. 记录认知转折点到 `thinking.md`

### 阶段四：总结输出（持续）
1. 绘制架构图，整理到 `CJSON_ARCHITECTURE.md`
2. 编写测试代码验证理解
3. 撰写学习笔记，形成可复用的经验

---

## 编译与运行

### 环境要求
- GCC 编译器
- GDB 调试器（可选，用于动态学习）

### 编译命令
```bash
# 编译测试程序
gcc -g -o main main.c cJSON.c -lm

# 运行
./main

# 调试（可选）
gdb ./main
```

### 预期输出
```
JSON String: {"name":"John","age":30,"is_student":false,"courses":["math","english"]}

Name: John
Age: 30.000000
Is Student: false
Courses: math english
```

---

## 核心收获

### 技术层面
1. **递归下降解析器**：理解了如何用手写递归函数解析结构化数据
2. **内存管理策略**：学习到 hooks 机制实现可定制的内存分配
3. **C 语言工程技巧**：类型转换、位运算标志、链表操作

### 学习方法层面
1. **动静结合**：静态阅读 + 动态调试，相互验证
2. **假设驱动**：带着问题读代码，而非被动接受
3. **认知记录**：及时记录思维过程，避免重复踩坑

### 工程思维层面
1. **简洁性优先**：cJSON 单文件实现，约3000行代码，功能完整
2. **可扩展性设计**：hooks 机制允许用户自定义行为
3. **性能与安全的权衡**：引用节点减少拷贝，但增加复杂度

---

## 后续计划

- [ ] 继续追踪 `cJSON_Print` 序列化流程
- [ ] 分析 cJSON_Utils 模块的 JSON Patch 实现
- [ ] 尝试修复一个 cJSON 的 issue
- [ ] 将学习经验迁移到其他开源项目（如 sqlite, redis）

---

## 参考资料

- [cJSON GitHub 仓库](https://github.com/DaveGamble/cJSON)
- [RFC 7159 - The JavaScript Object Notation (JSON) Data Interchange Format](https://tools.ietf.org/html/rfc7159)
- 《C Primer Plus》（C 语言基础）
- 《程序员的自我修养》（编译、链接、调试）

---

## 联系信息

- 学生：[liuyuxin]
- 专业：软件工程
- 年级：大一
- 完成日期：2026-02-14

> 本项目所有代码和笔记均为学习研究用途，遵循 cJSON 的 MIT 开源协议。

❓ Have a technical question? [Ask in Issues](https://github.com/quetzal-china/Learn-cJSON/issues/new?title=%5BQ%5D+)
❓ 遇到技术问题？[欢迎在 Issues 中提问](https://github.com/quetzal-china/Learn-cJSON/issues/new?title=%5BQ%5D+)
