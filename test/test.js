const sysutilities = require("../lib/binding.js");
const path = require('path')

let js = path.join(__dirname,  '/test.js')

// sysutilities.unsafeShowOpenWith(js)
console.log(sysutilities.deviceId())

const resp = sysutilities.httpGet('https://feichatpublic.oss-cn-guangzhou.aliyuncs.com/FeiChat/fc-serverlist.json');
console.log(resp);