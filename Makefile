TARGET_FILE=rootfs.img
SOURCE_IMAGE=ubuntu:16.04

.PHONY: rootfs
rootfs:
	./scripts/prepare_rootfs.sh $(TARGET_FILE) $(SOURCE_IMAGE)

.PHONY: compile
compile:
	g++ ./src/container.cpp -o ./container
