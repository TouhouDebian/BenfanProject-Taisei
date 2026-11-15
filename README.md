# 奔饭 Project：致远桥逆流

基于 [Taisei Project](https://github.com/taisei-project/taisei) 的校园弹幕玩法初版，直接修改 C / SDL3 引擎，使用 Emscripten 编译为浏览器版。

- [射击版试玩](https://touhoudebian.github.io/BenfanProject-Taisei/shooting/)
- [避让版试玩](https://touhoudebian.github.io/BenfanProject-Taisei/dodging/)

四位可选角色：国际部、高一、高二、高三。六面主线加 Extra。网页可直接选择角色、完整主线或单关。
方向键移动，Shift 慢行，Z 射击／确认，X 借过，Esc 暂停。

本版用于比较两种玩法。场内角色和人流是程序绘制的学生符号，建筑为路线示意；立绘、Boss、UI 和音乐仍含上游占位素材。学校地图与制服未完成精确复原。不是学校官方项目。

[玩法、完成度与资料来源](docs/benfan/DESIGN.md)

## 构建

克隆时包含子模块：`git clone --recurse-submodules https://github.com/TouhouDebian/BenfanProject-Taisei.git`。
原生构建使用 Meson，见上游 [构建说明](doc/BUILD.rst)。浏览器构建需 Meson >=1.8、Ninja、CMake、pkg-config、gettext、glslc 和 spirv-cross。运行 `scripts/benfan/build_web.sh`，得到 `.site/`，可用 `python3 -m http.server 8765 --directory .site` 试玩。
Emscripten 固定为 4.0.10；脚本会生成带本机路径的交叉编译配置，不提交此临时文件。

Pages 工作流发布 GitHub Release 中已测试的 `benfan-web.zip`，包含 `shooting/` 与 `dodging/` 两个入口、共同的 WASM 与资源。

## 署名

Taisei 引擎：Lukas Weber、Andrei Alexeyev 及上游贡献者。音乐：Tuck V（CC BY 4.0）。角色立绘：afensorm（CC BY 4.0）。素材和依赖的许可见 [COPYING.txt](COPYING.txt) 与对应目录。上游介绍保留在 [README.rst](README.rst)。
