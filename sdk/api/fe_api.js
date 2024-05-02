/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3
 *
 */
let mgpEarth;
FeApp = function() {}

function fe_onInitialize(app) {
    FeApp.onInitialize(mgpEarth);
}

function fe_onPickNode(app, path, name, index, properties) {
    if (mgpEarth.onPickNode) {
        /**
         * 鼠标拾取回调
         * @path 拾取对象的结点路径
         * @name 拾取到的对象名称
         * @indexOrId 拾取到的子对象索引信息。例如表示第几个子对象或者对象的ID。
         * @properties 属性信息，例如geojson的要素属性。
         */
        return mgpEarth.onPickNode(mgpEarth.Module.UTF8ToString(path), mgpEarth.Module.UTF8ToString(name),
            indexOrId, mgpEarth.Module.UTF8ToString(properties));
    }
    return true;
}

/**
 * 初始化mgpEarth模块
 * @canvasId 要显示的html canvas id
 * @w 显示宽带
 * @h 显示高度
 * @onInitialize 初始化后的回调
 */
mgpEarthInit = function(canvasId, w, h, onInitialize) {
    createMyModule({
        printErr: function() {
            console.warn(Array.prototype.slice.call(arguments).join(" "));
        },
        canvas: document.getElementById(canvasId),
    }).then(function(Module) {
        let app = new FeApp();
        FeApp.onInitialize = onInitialize;
        app.Module = Module;
        app.canvasId = canvasId;
        app.app = Module.ccall('fe_createApp', "number", ["number", "number"], 
            [w*window.devicePixelRatio, h*window.devicePixelRatio]);
            mgpEarth = app;
    });

    var canvas = document.getElementById(canvasId);
    fixGlfw(canvas);
}

/**
 * 设置显示canvas大小
 * @w 显示宽带
 * @h 显示高度
 */
FeApp.prototype.setCanvasSize = function(w, h) {
    this.Module.setCanvasSize(w*window.devicePixelRatio, h*window.devicePixelRatio);
    let canvas = document.getElementById(this.canvasId);
    canvas.style.width = w  + "px";
    canvas.style.height = h  + "px";
}

/**
 * 设置显示帧率等调试信息
 * @show 是否显示
 */
FeApp.prototype.showFps = function(show) {
    this.Module.ccall('fe_showFps', null, ["number","number"],
        [this.app, show]);
}

/**
 * 删除显示的对象
 * @name 对象名称，对应创建时设置的名称。
 */
FeApp.prototype.removeNode = function(name) {
    return this.Module.ccall('fe_removeNode', "number", ["number","string"],
        [this.app, name]);
}

/**
 * 增加瓦片图层
 * @name 图层对象名称，为了便于以后进行删除等操作
 * @uri 图层数据源地址
 * @elevationUri 地形数据源地址，可以为null。null表示全部0高度。
 * @options 选项json对象，可为空。有下列选项:
 *      minLevel: 最小数据层级，maxLevel最大数据层级，
 *      elevationMinLevel：高程数据最小层级，elevationMaxLevel：高程数据最小层级，elevationScale：高程缩放
 */
FeApp.prototype.addTileLayer = function(name, uri, elevationUri, options) {
    if (options) {
        options = JSON.stringify(options);
    }
    else {
        options = null;
    }
    this.Module.ccall('fe_addTileLayer', null, ["number","string","string","string", "string"],
        [this.app, name, uri, elevationUri, options]);
}

/**
 * 增加天空盒
 * @dark 是否为黑夜模式
 * @min 最小显示距离(camera离地面距离)
 * @max 最大显示距离(camera离地面距离)
 */
FeApp.prototype.addSkybox = function(dark, min, max) {
    this.Module.ccall('fe_addSkybox', null, ["number","number","number","number"],
        [this.app, dark, min, max]);
}

/**
 * 增加geojson图层
 * @name 图层对象名称，为了便于以后进行删除等操作
 * @uri 图层数据源地址
 * @options 选项json对象，可为空。有下列选项:
 *      maxDis: 最大显示距离(camera离地面距离), minDis：最小显示距离(camera离地面距离)
 *      height: 高程
 *      labelField：显示标注文字的字段
 *      outlineHeightOffset： 轮廓线高度偏移
 *      fillPolygon： 是否填充多边形
 *      strokePolygon： 多边形是否描边
 *      queryElevation： 是否自动查询高程
 *      lineStyle： 线样式 {depthTest： 是否深度测试，lineWidth： 线宽带度， dashLen：虚线长度，isFlowing： 流动效果，hasGlow：发光效果，lineColor：线颜色，flowColor：流动颜色}
 *      polygonStyle： 多边形样式 {depthTest： 是否深度测试， fillColor：填充颜色}
 *      labelStyle： 标注文字样式 {iconSize： 图标大小，fontSize：字体大小，iconImage： 图标图片路径，fontName： 字体名称，iconRect：图标在图片中的区域，
 *                      labelAlign： 图标和文字对齐方式， 0：文字在右边,1：文字填充图标，2：文字在下方，3：文字在上方。
 *                      fontColor： 文字颜色[r,g,b,a]， iconColor： 图标颜色[r,g,b,a]（会乘在图片颜色上）
 *                      sphereCulling: 是否通过法线剔除label, coverStrategy: 0:不允许相互压盖，1：允许压盖。
 *                  }
 */
FeApp.prototype.addGeoLayer = function(name, uri, options) {
    if (options) {
        options = JSON.stringify(options);
    }
    else {
        options = null;
    }
    this.Module.ccall('fe_addGeoLayer', null, ["number","string","string","string"],
        [this.app, name, uri, options]);
}

/**
 * 增加建筑物图层
 * @name 图层对象名称，为了便于以后进行删除等操作
 * @uri 图层数据源地址
 * @options 选项json对象，可为空。有下列选项:
 *      maxDis: 最大显示距离(camera离地面距离), minDis：最小显示距离(camera离地面距离)
 *      color: 颜色，依次为[r,g,b,a]，颜色值0-1之间。
 */
FeApp.prototype.addBuildingLayer = function(name, uri, options) {
    if (options) {
        options = JSON.stringify(options);
    }
    else {
        options = null;
    }
    this.Module.ccall('fe_addBuildingLayer', null, ["number","string","string","string"],
        [this.app, name, uri, options]);
}

/**
 * 增加3dtiles数据对象
 * @name 图层对象名称，为了便于以后进行删除等操作
 * @uri 图层数据源地址
 * @lng 经度
 * @lat 纬度
 * @height 高程（海拔高度）
 * @lighting 是否使用光照 
 * @options 选项json对象，可为空。有下列选项:
 *      rotateX: 绕X轴旋转，rotateY：绕Y轴旋转，rotateZ：绕Z轴旋转
 *      scale： 缩放
 */
FeApp.prototype.add3dtiles = function(name, uri, lng, lat, height, lighting, options) {
    if (options) {
        options = JSON.stringify(options);
    }
    else {
        options = null;
    }
    this.Module.ccall('fe_add3dtiles', null, ["number","string","string","number","number","number","number","string"],
        [this.app, name, uri, lng, lat, height, lighting, options]);
}

/**
 * 增加gltf模型对象
 * @name 图层对象名称，为了便于以后进行删除等操作
 * @uri 图层数据源地址
 * @lng 经度
 * @lat 纬度
 * @height 高程（海拔高度）
 * @lighting 是否使用光照 
 * @options 选项json对象，可为空。有下列选项:
 *      rotateX: 绕X轴旋转，rotateY：绕Y轴旋转，rotateZ：绕Z轴旋转
 *      scale： 缩放
 */
FeApp.prototype.addGroundGltf = function(name, uri, lng, lat, height, lighting, options) {
    if (options) {
        options = JSON.stringify(options);
    }
    else {
        options = null;
    }
    this.Module.ccall('fe_addGroundGltf', null, ["number","string","string","number","number","number","number","string"],
        [this.app, name, uri, lng, lat, height, lighting, options]);
}

/**
 * 增加灯光
 * @name 灯光对象名称，为了便于以后进行删除等操作
 * @uri 图层数据源地址
 * @lng 经度
 * @lat 纬度
 * @r 红色分量(0..1，可超过1)
 * @g 绿色分量(0..1，可超过1)
 * @b 蓝色分量(0..1，可超过1)
 */
FeApp.prototype.addLight = function(name, lng, lat, r, g, b) {
    this.Module.ccall('fe_addLight', null, ["number","string","number","number","number","number","number"],
        [this.app, name, lng, lat, r, g, b]);
}

/**
 * 设置Camera位置
 * @lng 经度
 * @lat 纬度
 * @zoom 缩放层级（0..20）
 */
FeApp.prototype.setPosition = function(lng, lat, zoom) {
    this.Module.ccall('fe_setPosition', null, ["number","number","number","number"],
        [this.app, lng, lat, zoom]);
}

/**
 * 动画移动Camera到指定位置
 * @lng 经度
 * @lat 纬度
 * @time 动画时间，单位毫秒
 * @zoom 缩放层级（0..20）
 */
FeApp.prototype.moveTo = function(lng, lat, time, zoom) {
    if (zoom === undefined) {
        zoom = NaN;
    }
    this.Module.ccall('fe_moveTop', null, ["number","number","number","number","number"],
        [this.app, lng, lat, time, zoom]);
}

/**
 * 缩放到指定层级
 * @zoom 缩放层级（0..20）
 * @time 动画时间，单位毫秒
 */
FeApp.prototype.zoomTo = function(zoom, time) {
    this.Module.ccall('fe_zoomTo', null, ["number","number","number"],
        [this.app, zoom, time]);
}

/**
 * 旋转Camera
 * @rx 旋转俯仰角
 * @ry 旋转方位角
 * @time 动画时间，单位毫秒
 */
FeApp.prototype.rotateTo = function(rx, rz, time) {
    this.Module.ccall('fe_rotateTo', null, ["number","number","number","number"],
        [this.app, rx, rz, time]);
}

/**
 * 添加多实例渲染模型
 * @name 图层对象名称，为了便于以后进行删除等操作
 * @uri 图层数据源地址（gltf模型文件）
 * @lighting 是否使用光照 
 * @options 选项json对象，可为空。
 */
FeApp.prototype.addMultiModel = function(name, uri, lighting, options) {
    if (options) {
        options = JSON.stringify(options);
    }
    else {
        options = null;
    }
    this.Module.ccall('fe_addMultiModel', null, ["number","string","string","number","string"],
        [this.app, name, uri, lighting, options]);
}

/**
 * 为多实例渲染模型增加实例
 * @name 多实例模型的名称。
 * @instId 实例ID，用户自定义的整形。如果已存在则修改，如果不存在则新建实例。
 * @lng 经度
 * @lat 纬度
 * @height 高程（海拔高度）
 * @options 选项json对象，可为空。有下列选项:
 *      rotateX: 绕X轴旋转，rotateY：绕Y轴旋转，rotateZ：绕Z轴旋转
 *      scale： 缩放
 *      speed: 移动速度， path: 移动路径(经纬度坐标串)
 */
FeApp.prototype.updateModelInstance = function(name, instId, lng, lat, height, options) {
    if (options) {
        options = JSON.stringify(options);
    }
    else {
        options = null;
    }
    return this.Module.ccall('fe_updateModelInstance', "number", ["number","string","number","number","number","number","string"],
        [this.app, name, instId, lng, lat, height, options]);
}

/**
 * 删除多实例渲染模型的一个实例
 * @name 多实例模型的名称
 * @instId 要删除的实例ID
 */
FeApp.prototype.removeModelInstance = function(name, instId) {
    if (options) {
        options = JSON.stringify(options);
    }
    else {
        options = null;
    }
    this.Module.ccall('fe_removeModelInstance', null, ["number","string","number"],
        [this.app, name, instId]);
}

/**
 * 创建新的几何图层
 * @name 图层名称
 * @geotype 几何类型：（Point：1，LineString：3，Polygon：5）
 * @options 见addGeoLayer的options参数
 */
FeApp.prototype.addEmptyGeoLayer = function(name, geotype, options) {
    if (options) {
        options = JSON.stringify(options);
    }
    else {
        options = null;
    }
    this.Module.ccall('fe_addEmptyGeoLayer', null, ["number","string","number","string"],
        [this.app, name, geotype, options]);
}

/**
 * 为几何图层增加对象
 * @name 图层名称
 * @geotype 几何类型：（Point：1，LineString：3，Polygon：5）
 * @coords 坐标数组，Float32Array类型，格式[经度，维度，高度， 经度，维度，高度,...]
 * @options 见addGeoLayer的options参数
 */
FeApp.prototype.addGeoFeature = function(name, geotype, coords, attributes) {
    if (attributes) {
        attributes = JSON.stringify(attributes);
    }
    else {
        attributes = null;
    }
    let pointNum = coords.length / 3;
    let buf = new Int8Array(coords.buffer);

    let res = this.Module.ccall('fe_addGeoFeature', "number", ["number","string","number", "array", "number", "string"],
        [this.app, name, geotype, buf, pointNum, attributes]);
    return res;
}

/**
 * 删除几何图层的对象
 * @name 图层名称
 * @fieldName 字段名
 * @value 删除等于此值的对象
 * @return 返回删除对象的个数
 */
FeApp.prototype.removeGeoFeature = function(name, fieldName, value) {
    return this.Module.ccall('fe_removeGeoFeature', "number", ["number","string","string","string"],
        [this.app, name, fieldName, value]);
}

/**
 * 获取数据下载进度
 * @name 图层名称
 * @return 返回0-1，1表示加载完成。
 * 注意：如果是gltf数据可能只是下载进度，不包括解析进度。
 */
FeApp.prototype.getLoadProgress = function(name) {
    return this.Module.ccall('fe_getLoadProgress', "number", ["number","string"],
        [this.app, name]);
}

/**
 * 同步调用拾取接口
 * @param {*} name 需要拾取的对象名称，传null表示在所有对象中拾取
 * @param {*} x 屏幕坐标x，单位物理像素，非dp像素
 * @param {*} y 屏幕坐标y，单位物理像素，非dp像素
 * @returns 如果有拾取结果返回[对象名称,[lng,lat,height]]，拾取失败返回null.
 */
FeApp.prototype.syncPick = function(name, x, y) {
    let outName = Module._malloc(256);
    let outCoord = Module._malloc(24);
    let rc = this.Module.ccall('fe_syncPick', "number", ["number","string","number","number", "number", "number"],
        [this.app, name, x, y, outName, outCoord]);

    let res = null;
    if (rc) {
        let x = this.Module.HEAPF64[outCoord/8];
        let y = this.Module.HEAPF64[outCoord/8+1];
        let z = this.Module.HEAPF64[outCoord/8+2];

        res = [this.Module.UTF8ToString(outName),
            this.xyzToBl(x, y, z)
        ];
    }

    this.Module._free(outName);
    this.Module._free(outCoord);
    return res;
}

/**
 * 空间直角坐标系转经纬度
 */
FeApp.prototype.xyzToBl = function(x, y, z) {
    let outCoord = this.Module.ccall('fe_xyzToBl', "number", ["number","number","number", "number", "number"],
        [this.app, x, y, z, null]);

    let res = [ 
                this.Module.HEAPF64[outCoord/8],
                this.Module.HEAPF64[outCoord/8+1],
                this.Module.HEAPF64[outCoord/8+2]
            ];
    return res;
}

/**
 * 纬度转空间直角坐标系经
 */
FeApp.prototype.blToXyz = function(lng, lat, height) {
    let outCoord = this.Module.ccall('fe_blToXyz', "number", ["number","number","number", "number", "number"],
        [this.app, lng, lat, height, null]);

    let res = [ 
                this.Module.HEAPF64[outCoord/8],
                this.Module.HEAPF64[outCoord/8+1],
                this.Module.HEAPF64[outCoord/8+2]
            ];
    return res;
}

/**
 * 清除高亮
 */
FeApp.prototype.clearHighlight = function() {
    this.Module.ccall('fe_clearHighlight', null, ["number"],
        [this.app]);
}

