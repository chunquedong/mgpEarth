

## 开始使用

1.增加fastearth代码链接
```
<script type="text/javascript" src="fastEarthApi.js"></script>
```

2.在html内声明canvas对象作为绘图容器
```
<canvas class="emscripten" id="canvas" oncontextmenu="event.preventDefault()"></canvas>
```

3.加载fasetearth
```
      let w = window.innerWidth;
      let h = window.innerHeight;
      fastEarthInit('canvas', w, h, function(fastEarth){
          //增加天空盒
          fastEarth.addSkybox(1, 20000, Number.MAX_VALUE);
          fastEarth.addSkybox(0, 0, 20000);

          //增加影像图层
          let elevationUri = "/terrain/{z}/{x}/{y}.pngraw?access_token=pk.eyJ1IjoicnpvbGxlciIsImEiOiIzQ1V3clI4In0.2TF5_QTXSR3T7F_dyPd1rg";
          fastEarth.addTileLayer("gd", "/gd?style=6&x={x}&y={y}&z={z}", elevationUri);

          //设置初始位置
          fastEarth.setPosition(108.964164, 34.218175, 0);
      });
```

4.配置路径
需要修改server/index.js内部的文件路径到下载的文件位置:
```
const cachePath = "C:/fastearth-sdk/cache";
const codePath = "C:/fastearth-sdk/fastearth";
const dataPath = "C:/fastearth-sdk/data";
```

5.启动后端服务
```
cd server
npm start
```

### 了解更多
- API文档见sdk/api/fe_api.js
- 示例代码：sdk/demo/
