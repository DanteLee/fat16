var append = require('fs').appendFile;
var del = require('fs').unlinkSync;
var size = parseInt(process.argv[2]);
var buffer = new Buffer(2048);

try {
    del('disk.flp');
} catch (e) {}

buffer.fill(0);
for (var i = 0; i < (size * 1024 * 1024) / 2048; i++) {
    append('disk.flp', buffer);
}
