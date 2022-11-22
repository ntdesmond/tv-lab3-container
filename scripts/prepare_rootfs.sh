#!/bin/bash

set -e

TARGET_FILE=$1
DOCKER_IMAGE=$2
DESCRIPTION="Creates an ext4 image with filesystem extracted from a Docker image"

if (( $# < 2 ))
then
  echo -e "Usage: $0 TARGET_FILE DOCKER_IMAGE\n\n$DESCRIPTION\n"
  exit
fi

# Delete old rootfs file, if any
rm -f $TARGET_FILE

# Pull image and create docker container
docker pull $DOCKER_IMAGE
DOCKER_CONTAINER_HASH=$(docker create $DOCKER_IMAGE)
echo "Created a temporary Docker container"

# Prepare the target file
echo "Creating the rootfs file... "
dd if=/dev/zero of=$TARGET_FILE bs=100M count=10
mkfs.ext4 $TARGET_FILE

# Mount the target
MOUNT_DIR=/tmp/rootfs_$DOCKER_CONTAINER_HASH
mkdir -p $MOUNT_DIR
mount -o loop $TARGET_FILE $MOUNT_DIR
echo "File created and mounted to $MOUNT_DIR"

# Extract the filesystem
docker export $DOCKER_CONTAINER_HASH | tar -C $MOUNT_DIR -xf -
echo "Filesystem extracted to '$MOUNT_DIR'"

# Remove the docker container
docker rm $DOCKER_CONTAINER_HASH > /dev/null
echo "Removed the Docker container"

# Unmount the target
umount $MOUNT_DIR
rmdir $MOUNT_DIR
echo "Unmounted $TARGET_FILE, the file is ready"
