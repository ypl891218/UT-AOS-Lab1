#!/bin/bash

# 1. generate ssh key
ssh-keygen -t rsa -b 4096

# 2. generate cloud.txt
cat > cloud.txt << EOF
#cloud-config
users:
  - name: yunping
    ssh-authorized-keys:
      -
EOF
cat ~/.ssh/id_rsa.pub >> cloud.txt
cat >> cloud.txt << EOF
    sudo: ['ALL=(ALL) NOPASSWD:ALL']
    groups: sudo
    shell: /bin/bash
EOF

# 3. Remember to remove the newline before ssh key

# 4. make cloud.img
sudo apt -y install cloud-image-utils
cloud-localds cloud.img cloud.txt

# 5. wget ubuntu img
if [ ! -f ubuntu-20.04-server-cloudimg-amd64.img ]; then
	wget https://cloud-images.ubuntu.com/releases/focal/release-20240821/ubuntu-20.04-server-cloudimg-amd64.img
fi

# 6. cp backup
cp ubuntu-20.04-server-cloudimg-amd64.img ubuntu-20.04-server-cloudimg-amd64.img.bkp

# 7. boot VM!
sudo qemu-system-x86_64 \
    -enable-kvm \
    -smp 2 \
    -m 1024 \
    -nographic \
    -hda ubuntu-20.04-server-cloudimg-amd64.img \
    -hdb cloud.img \
    -netdev user,id=net0,hostfwd=tcp::2222-:22 \
    -device virtio-net-pci,netdev=net0

