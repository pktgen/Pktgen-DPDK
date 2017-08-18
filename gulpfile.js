var cp = require('child_process');
var gulp = require('gulp');
gulp.task('default', function () {
    var sdk = global.process.env.RTE_SDK;
    var target = process.env.RTE_TARGET || 'x86_64-native-linuxapp-gcc';
//    var cmd = 'make -j EXTRA_CFLAGS=\"-g -O3\"';
    var cmd = 'env RTE_SDK=' + sdk + ' RTE_TARGET=' + target + ' make -j';
    console.log('Build using:' + cmd);
    cp.execSync(cmd, {
        stdio: 'inherit'
    });
});
gulp.task('debug-build', function () {
    var env = process.env.RTE_TARGET || 'x86_64-native-linuxapp-gcc';
    var cmd = 'make -j EXTRA_CFLAGS=\"-g -O0\"';
    console.log('Build using:' + cmd);
    cp.execSync(cmd, {
        stdio: 'inherit'
    });
});
