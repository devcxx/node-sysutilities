const sysutilities = require("../lib/binding.js");
const path = require('path')

let js = path.join(__dirname,  '/test.js')

// sysutilities.unsafeShowOpenWith(js)
console.log(sysutilities.deviceId())