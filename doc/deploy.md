
## 部署

由于使用了SharedArrayBuffer特性，所以部署时需要特殊配置。

#### 设置跨域隔离
不同的服务有不同的设置方法。

Node.js:
```
res.header("Cross-Origin-Embedder-Policy", "require-corp");
res.header("Cross-Origin-Opener-Policy", "same-origin");
```

nginx 配置:
```
location / {
    add_header 'Cross-Origin-Embedder-Policy' 'require-corp';
    add_header 'Cross-Origin-Opener-Policy' 'same-origin';
}
```

#### 配置HTTPS
有三种方式允许使用SharedArrayBuffer：
- 使用https协议
- 只能使用localhost域。
- 在chrome://flags页面增加白名单。

