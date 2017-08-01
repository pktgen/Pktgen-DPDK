var cp = require('child_process');
var gulp = require('gulp');
gulp.task('build', function () {
    var env = process.env.RTE_TARGET || 'x86_64-native-linuxapp-gcc';
    var cmd = 'make -j EXTRA_CFLAGS=\"-g -O3\"';
    console.log('Build using:' + cmd);
    cp.execSync(cmd, {
        stdio: 'inherit'
    });
});
gulp.task('debug', function () {
    var env = process.env.RTE_TARGET || 'x86_64-native-linuxapp-gcc';
    var cmd = 'make -j EXTRA_CFLAGS=\"-g -O0\"';
    console.log('Build using:' + cmd);
    cp.execSync(cmd, {
        stdio: 'inherit'
    });
});