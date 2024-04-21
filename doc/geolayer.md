

## 地理矢量图层

### 概述
用来显示点、线、面等数据。

### 从Geojson文件加载

通过addGeoLayer加载已有的geojson文件。
```
  //最后的options参数用来配置图层显示样式。
  mgpEarth.addGeoLayer("polygon", "https://d2ad6b4ur7yvpq.cloudfront.net/naturalearth-3.3.0/ne_110m_admin_1_states_provinces_shp.geojson", {
    "queryElevation" : true,
    "lineStyle": {
      "lineWidth": 1,
      "depthTest": true
    },
    "polygonStyle": {
      "fillColor": [0.5,0.7,0.6,0.6],
    }
  });

```

### 代码创建图层

通过addEmptyGeoLayer来增加图层，addGeoFeature对其增加数据。

点状数据：
```
    //创建名称为poi的图层。1表示点状数据。
    mgpEarth.addEmptyGeoLayer("poi", 1, {
        "labelStyle": {
            "iconSize": 60,
            "iconImage": "res/image/m2.png",
            "labelAlign": 1,
            "iconColor": [1.0, 0.0, 0.0, 1.0],
            "sphereCulling": false
        }
    });

    //为poi图层增加点对象， 1表示几何类型为点。
    //坐标为Float32Array类型的经纬度和高程的列表。
    //最后的属性会忽略字段数据类型，内部统一用字符串存储。
    mgpEarth.addGeoFeature("poi", 1, new Float32Array([108.964164, 34.218175, 100]), {id:"123",name:"test1"} );
    mgpEarth.addGeoFeature("poi", 1, new Float32Array([108.964164+0.1, 34.218175, 100]), {id:"124",name:"test2"} );

    //删除其中id=123的点
    mgpEarth.removeGeoFeature("poi", "id", "123");
```

多边形数据：
```
    //创建名称为line的图层。5表示面状数据。
    mgpEarth.addEmptyGeoLayer("line", 5, {
            //"queryElevation" : true,
            "lineStyle": {
              "lineWidth": 3,
              "depthTest": true
            },
            "polygonStyle": {
              "fillColor": [0.5,0.7,0.6,0.6]
            },
            "outlineHeightOffset": 0,
            "labelStyle": {
              "sphereCulling": false
            }
    });

    //为poi图层增加点对象， 5表示几何类型为面。 面坐标的起始点必须相同。
    mgpEarth.addGeoFeature("line", 5, new Float32Array([
            108.964164, 34.218175, 500, 
            108.964164+0.1, 34.218175, 500, 
            108.964164, 34.218175+0.1, 500,
            108.964164, 34.218175, 500
    ]), {id:"123",name:"test1"} );
```


修改数据：目前没有专用的接口，可以通过先删除再添加的方式实现。


删除图层: mgpEarth.removeNode("poi")


### 点击拾取

通过onPickNode回调。name是图层名称，properties是属性信息。
```
    mgpEarth.onPickNode = function(path, name, index, properties) {
        console.log("pick:"+name+"/"+index+": "+properties);
        return true;
    };
```