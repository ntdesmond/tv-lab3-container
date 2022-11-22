TARGET_FILE=rootfs.img
SOURCE_IMAGE=busybox

.PHONY: rootfs
rootfs:
	./scripts/prepare_rootfs.sh $(TARGET_FILE) $(SOURCE_IMAGE)

.PHONY: compile
compile:
	g++ ./src/container.cpp -o ./container

.PHONY: restore
restore:
	mount -t proc proc /proc
	umount -l rootfs
	losetup -d $$(losetup -a | grep $(TARGET_FILE) | cut -d':' -f 1)

.PHONY: shell
shell:
	./container /bin/sh

.PHONY: run
run: shell restore
