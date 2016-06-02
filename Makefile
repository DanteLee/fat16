test-disk: src/disk.c src/util.c test/disk.c
	gcc -g src/util.c src/disk.c test/disk.c -I src/ -o out/test-disk

test-time: src/dostime.c test/dostime.c
	gcc -g src/dostime.c test/dostime.c -I src/ -o out/test-time

test-fs: src/disk.c src/util.c src/dostime.c src/fs.c test/main.c
	gcc -g src/disk.c src/util.c src/dostime.c src/fs.c test/main.c -I src/ -o out/test-fs -lm
