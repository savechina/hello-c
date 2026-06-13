// 1. 动态创建并配置远程 <script> 标签
var gaScript = document.createElement("script");
gaScript.async = true;
gaScript.src = "https://www.googletagmanager.com/gtag/js?id=G-JBDQHSMGWL"; // Site ID

// 2. 将创建好的标签插入到页面的 <head> 头部中
document.head.appendChild(gaScript);

// 3. 初始化 Google Analytics 的全局数据层和配置
window.dataLayer = window.dataLayer || [];
function gtag() {
  dataLayer.push(arguments);
}

gtag("js", new Date());
gtag("config", "G-JBDQHSMGWL"); //  Site ID
