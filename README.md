# obs-replay-buffer-confirm

给 OBS 的回放缓存加一道二次确认。停止回放缓存会把缓冲内容一起丢掉，误点一次没法挽回。

## 支持版本

OBS Studio 32.2.2。头文件与接口按 32.2.2 编译，33.x 未验证。

## 安装

下载 Release 里的 zip，解压到 `C:\ProgramData\obs-studio\plugins\`，目录结构应为：

```
plugins\obs-replay-buffer-confirm\bin\64bit\obs-replay-buffer-confirm.dll
plugins\obs-replay-buffer-confirm\data\locale\zh-CN.ini
```

重启 OBS 生效。便携模式下这个目录不参与加载。

## 使用

设置 → 常规 → 输出，勾选「关闭回放缓存时弹窗确认」，位置在「停止录制时弹窗确认」下方。
开启后点主界面的「停止回放缓存」，或系统托盘里的同名项，会先弹确认框，默认停在「否」。
主窗口最小化到托盘时不弹窗，直接停止，和 OBS 的停止录制确认一致。

开关存在 `%AppData%\obs-studio\plugin_config\obs-replay-buffer-confirm\settings.json`。

## 没有覆盖的入口

- 热键 `OBSBasic.StopReplayBuffer`：由 OBS 自己注册，插件拿不到那个回调
- 停止录制或停止推流带出的自动停止：属于主操作的结果，不再打断一次

## 构建

见 `AGENTS.md`。

## 许可

GPL-3.0，见 `LICENSE`。