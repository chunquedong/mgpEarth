

## 从源码构建

预构建的SDK可以从官网下载。自己编译会比较麻烦。

### 需要安装的软件
- JDK
- Python 3
- Node.js
- [Emscripten](https://emscripten.org/docs/getting_started/downloads.html)
- [Fanx](https://github.com/fanx-dev/fanx/releases)
- [fmake](https://github.com/)

### 依赖的第三方库
- mgpCore
- mgpModules
- jsonc
- draco
- mgpPro

桌面版本还依赖下列库:
- glfw
- glew
- miniaudio
- bullet
- freetype
- curl
- zlib

请联系作者获取预编译的第三方库。

### 构建Web版
需要先配置fmake配置文件，见fmake项目的readme文件中的Emscripten配置。


编译Web版
```
sh build.sh
```

### 构建桌面版

编译
```
fan famke src/fmake.props -debug
```
生成IDE项目文件
```
fan famke src/fmake.props -debug -G
```

