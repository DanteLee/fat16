test-disk: src/disk.c src/util.c test/disk.c
	gcc -g src/util.c src/disk.c test/disk.c -I src/ -o out/test-disk
