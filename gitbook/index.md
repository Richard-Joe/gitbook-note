# gitbook搭建


## 安装 npm

```
官网地址：https://nodejs.org/en/

windows平台会有一些错误问题，我选择了v12.18版本
https://npm.taobao.org/mirrors/node/v12.18.4/node-v12.18.4-x64.msi
```


## 安装 gitbook

```
npm install -g gitbook-cli

gitbook --version
```


## 初始化

```
cd gitbook-note
 gitbook init
```


## 启动本地服务

```
gitbook serve

浏览器访问http://localhost:4000/
```


## 编译成 html

```
npm init -y

在package.json中修改scripts加上build
"scripts": {
    "build": "gitbook build . ./docs"
}

npm run build
```


## 部署到 Github Pages

推送到 github ，然后到 Github Pages 一项下面。Source 一项设置为 main，/docs 

然后就可以访问https://richard-joe.github.io/gitbook-note/


## 错误参考

可能遇到的错误，比如：


1. 脚本报错
```
C:\Users\frank\AppData\Roaming\npm\node_modules\gitbook-cli\node_modules\npm\node_modules\graceful-fs\polyfills.js:287

if (cb) cb.apply(this, arguments)
```
解决方法：
打开polyfills.js文件，把调用statFix的地方注释掉。
//fs.stat = statFix(fs.stat)
//fs.fstat = statFix(fs.fstat)
//fs.lstat = statFix(fs.lstat)


2. gitbook init 错误
```
> gitbook init
warn: no summary file in this book
info: create README.md
info: create SUMMARY.md

TypeError [ERR_INVALID_ARG_TYPE]: The "data" argument must be of type string or an instance of Buffer, TypedArray, or DataView. Received an instance of Promise
```
解决方法：
选择低版本的node，
https://stackoverflow.com/questions/61538769/gitbook-init-error-typeerror-err-invalid-arg-type-the-data-argument-must-b
