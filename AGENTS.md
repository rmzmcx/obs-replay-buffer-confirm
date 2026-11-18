# 协作规则（AGENTS.md）

本文件记录在这个工作区 / 仓库里协作时必须遵守的约定，供以后的会话（包括其它 AI）读取。
（Windows 文件名不区分大小写，写 `agent.md` 读到的也是本文件；这里用 `AGENTS.md` 是因为
这类工具默认会自动查找这个名字。）
书面写作规则：

When you are writing, please

- be brief
- reduce concluding sentences
- reduce parallel words
- reduce semicolons: adversative and progressive sentences should use correct and various conjunctions
- reduce negatively emphasising semantics like "..., not ..." / "..., rather than ..." / "..., without ..."
- stop reasserting a certain rule
- reduce abuse of semantics like "retain" / "remain" / "keep"
- reduce abuse of identical sentence structures
- reduce bold or highlighting
- stop using non-UTF8 symbols
- prevent a mechanical feel
- be nature and more human-like

---

代码工作与问题解决规则：

- 回答要简洁且突出重点，步骤要明晰，不要重复总结。
- 遇到不确定的情况时积极联网查找信息，不要猜测。
- 修改代码时严格检查之前对同一个任务做过的所有工作，从整体上优化算法，能合并的就合并，不要把功能堆成屎山。

---

文档清洁规则：

- 不要把不相关的内容加到各点下。
- 不要重复同一内容以强调。
- 不要把提示词和你的思考过程作为规则加入文档内强调。
- 不要把文档和代码堆成屎山。
- 每次文档生成或修改完毕后按此清洁规则做文档清洁，以减少屎山的堆积。

---

代码清洁规则：

- 更新算法时，旧框架应当 prune。导致的无用代码可删除，不需要保留。
- 旧方案除无法迁移到新方案的部分，应当全部优化。后续若新方案的功能完成，框架应当被 prune。
- 同类文件过多时，应当合并同类项，减少复杂的嵌套调用。
- 每次代码生成或修改完毕后按此清洁规则做代码清洁，以减少屎山的堆积。

## 工作区与仓库

- 工作区：`E:\DSH-Work\OBS插件`
- 主仓库： `E:\DSH-Work\OBS插件\Github\obs-replay-buffer-confirm`
- **只做本地 commit。** push、打 tag、发 GitHub release 都由用户本人完成，不要推送、不要动远端。
- obs 参考仓库：`E:\DSH-Work\OBS插件\Github\obs-studio`，只用来参考 / 查常量，**不要修改**。
## 目录与发布规则（用户明确要求）
1. commit 要写**说明**，写清「为什么」
   **每次修改后先等用户确认，确认之后再提交本地 commit。**

## 构建 / 部署 流程

依赖放在仓库的 `.deps/`，不入库：obs 32.2.2 头文件快照、由 OBS 自带 DLL 生成的导入库、Qt 6.11.1 开发包。

路径含中文时 moc 与 cl 会读错编码，构建前先映射一个 ASCII 盘符：

```
cmd /c subst X: "E:\DSH-Work\OBS插件"
```

配置、构建、安装（CMake 用 VS 自带的那份）：

```
& 'E:\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' -S X:\Github\obs-replay-buffer-confirm -B X:\Github\obs-replay-buffer-confirm\build -G "Visual Studio 18 2026" -A x64 -DCMAKE_PREFIX_PATH=X:\Github\obs-replay-buffer-confirm\.deps\qt6
& <同上 cmake> --build X:\Github\obs-replay-buffer-confirm\build --config RelWithDebInfo
& <同上 cmake> --install X:\Github\obs-replay-buffer-confirm\build --config RelWithDebInfo --prefix X:\Github\obs-replay-buffer-confirm\release
```

部署：把 `release\obs-replay-buffer-confirm` 整个复制到 `C:\ProgramData\obs-studio\plugins\`，重启 OBS 生效。
调试：OBS 安装目录的 `bin\64bit\obs64.exe` 作为调试目标，工作目录设为 `bin\64bit`。