
## 3D模型加载

支持gltf和3dtiles格式。

添加图层/模型接口第一个参数为对象名称，用于后续修改和删除操作，名称请勿重复。

### 单个gltf加载
```
    //options参数用于调整模型的默认姿态
    mgpEarth.addGroundGltf("gltf", "/data/shanghai/scene.gltf",
        121.507762+0.006, 31.233975-0.003, 118, true, {
        "rotateX" : 3.1415926/2,
        "rotateY" : 245/360.0*3.1415926*2,
        "scale": 520
    });

    //场景中添加灯光，颜色可以使用大于1.0的值。
    mgpEarth.addLight("light", 140, 0, 28, 28, 28);
```

### 加载3dtiles
```
    //最后的options参数和addGroundGltf类似，用于调整模型的默认姿态，这里使用默认值。
    mgpEarth.add3dtiles("da_yan_ta", "/3dtiles/qx-dyt/0/root.json", 108.964164, 34.218175, 35, false);
```

### 多实例模型

使用addGroundGltf加载的gltf模型，多个实例不能共享资源。为提高效率使用addMultiModel来实现多实例支持。
```
    //加载gltf模型
    mgpEarth.addMultiModel("car", "/data/gltf/seatLeon.gltf", false, null);

    //创建car模型的第一个实例
    mgpEarth.updateModelInstance("car", -1, 108.964164, 34.218175, 0, {
        "rotateX" : -3.1415926/2,
        "rotateZ" : 3.1415926,
        "scale": 10
    });

    //创建car模型的第二个实例
    mgpEarth.updateModelInstance("car", -1, 108.964164, 34.218175, 0, {
        "rotateX" : -3.1415926/2,
        "rotateZ" : 3.1415926,
        "scale": 10
    });
```
