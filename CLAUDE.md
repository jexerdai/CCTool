# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目简介

CCTool 是一个 Claude Code 的图形化工作站，用 C++ + Qt 开发，类似 SourceTree 管理 Git 仓库的方式管理多个 CC 仓库，提供更好的交互体验和历史记录功能。

## 技术栈

- **语言：** C++17
- **UI 框架：** Qt 6
- **数据库：** SQLite（通过 Qt SQL 模块）
- **构建系统：** CMake

## 构建方式

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=<Qt6安装路径>
cmake --build .
```

## 目录结构

```
CCTool/
├── CMakeLists.txt
├── Sources/
│   ├── Main.cpp
│   ├── MainWindow.h/.cpp       主窗口（QSplitter + QTabWidget）
│   ├── Ui/                     纯 UI 组件层
│   │   ├── LeftPanel           仓库列表面板
│   │   ├── RepoTab             单个仓库 Tab 页（串联所有组件）
│   │   ├── OutputView          CC 输出显示（QTextBrowser）
│   │   ├── RichTextEditor      富文本输入框（Ctrl+Enter 发送）
│   │   └── HistoryPanel        历史会话列表
│   ├── Core/                   业务逻辑层
│   │   ├── CCProcess           QProcess 驱动 claude CLI + JSON 解析
│   │   └── StreamSimulator     QTimer 模拟流式逐字显示
│   └── Data/                   数据层
│       ├── Database            SQLite CRUD（单例）
│       ├── RepoInfo.h          仓库结构体
│       ├── SessionInfo.h       会话结构体
│       └── Message.h           消息结构体
├── Resources/
│   ├── App.qrc
│   └── Styles/Main.qss         暗色主题样式
└── Scripts/
    └── FixSln.ps1              修正 .sln 路径后输出到根目录
```

## 与 Claude Code 的通信方式

通过 `QProcess` 驱动 Claude Code CLI 的非交互模式：

```bash
# 新会话
claude -p "<prompt>" --output-format json

# 继续已有会话
claude -p "<prompt>" --output-format json --resume <session-id>
```

返回结构化 JSON，解析后显示到 OutputView，session_id 存入 SQLite 以维持多轮对话。

## 数据库结构

```sql
CREATE TABLE repos    (id, name, path, created_at);
CREATE TABLE sessions (id, repo_id, cc_session_id, created_at);
CREATE TABLE messages (id, session_id, role, content, created_at);
```

## 开发阶段

- **Phase 1：** 框架骨架 ✅（全部文件已创建）
- **Phase 2：** 调试 CCProcess 与真实 claude CLI 的 JSON 通信
- **Phase 3：** 输入体验完善（RichTextEditor Markdown 预览等）
- **Phase 4：** 历史记录完善（消息回溯、多会话管理）
