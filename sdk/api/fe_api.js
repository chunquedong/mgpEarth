/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3
 *
 */
let fastEarth;
FeApp = function() {}

function fe_onInitialize(app) {
    FeApp.onInitialize(fastEarth);
}

function fe_onPickNode(app, path, name, index, properties) {
    if (fastEarth.onPickNode) {
        return fastEarth.onPickNode(fastEarth.Module.UTF8ToString(path), fastEarth.Module.UTF8ToString(name),
             index, fastEarth.Module.UTF8ToString(properties));
    }
    return true;
}

fastEarthInit = function(canvasId, w, h, onInitialize) {
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
        fastEarth = app;
    });
}

FeApp.prototype.setCanvasSize = function(w, h) {
    this.Module.setCanvasSize(w*window.devicePixelRatio, h*window.devicePixelRatio);
    let canvas = document.getElementById(this.canvasId);
    canvas.style.width = w  + "px";
    canvas.style.height = h  + "px";
}

FeApp.prototype.showFps = function(show) {
    this.Module.ccall('fe_showFps', null, ["number","number"],
        [this.app, show]);
}

FeApp.prototype.removeNode = function(name) {
    return this.Module.ccall('fe_removeNode', "number", ["number","string"],
        [this.app, name]);
}

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

FeApp.prototype.addSkybox = function(dark, min, max) {
    this.Module.ccall('fe_addSkybox', null, ["number","number","number","number"],
        [this.app, dark, min, max]);
}

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

FeApp.prototype.addLight = function(name, lng, lat, r, g, b) {
    this.Module.ccall('fe_addLight', null, ["number","string","number","number","number","number","number"],
        [this.app, name, lng, lat, r, g, b]);
}

FeApp.prototype.setPosition = function(lng, lat, zoom) {
    this.Module.ccall('fe_setPosition', null, ["number","number","number","number"],
        [this.app, lng, lat, zoom]);
}

FeApp.prototype.moveTo = function(lng, lat, time, zoom) {
    if (zoom === undefined) {
        zoom = NaN;
    }
    this.Module.ccall('fe_moveTop', null, ["number","number","number","number","number"],
        [this.app, lng, lat, time, zoom]);
}

FeApp.prototype.zoomTo = function(zoom, time) {
    this.Module.ccall('fe_zoomTo', null, ["number","number","number"],
        [this.app, zoom, time]);
}

FeApp.prototype.rotateTo = function(rx, rz, time) {
    this.Module.ccall('fe_rotateTo', null, ["number","number","number","number"],
        [this.app, rx, rz, time]);
}
