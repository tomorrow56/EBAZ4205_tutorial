# PetaLinuxの実行で使用したコマンドリスト

## 必要なパッケージのインストール

```bash
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install iproute2 gawk python3 python-is-python3 build-essential gcc git make net-tools libncurses-dev tftpd-hpa zlib1g-dev libssl-dev flex bison libselinux1 gnupg wget diffstat chrpath socat xterm autoconf libtool tar unzip texinfo gcc-multilib automake zlib1g:i386 screen pax gzip cpio python3-pip python3-pexpect xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa-dev libsdl1.2-dev pylint
```

## PetaLinuxのインストール

```bash
chmod 755 ./petalinux-v2024.2-11062026-installer.run
mkdir -p /home/user/petalinux2024.2
./petalinux-v2024.2-11062026-installer.run --dir /home/user/petalinux2024.2

source /home/user/petalinux2024.2/settings.sh
echo $PETALINUX
```


## テストプロジェクトのビルド

```bash
cd /home/user/petalinux2024.2
petalinux-create project -s xilinx-zc702-v2024.2-11110212.bsp

cd xilinx-zc702-2024.2
petalinux-config

petalinux-build

ls -la images/linux/
```

## コラム: Windows11のWSL2のUbuntu 24.04で実行する

PowerShell
```powershell
wsl --list --online

wsl --install -d Ubuntu-24.04

lsb_release -a
```

Ubuntu
```bash
sudo apt update
sudo apt install dnsutils

cd /tmp
wget http://security.ubuntu.com/ubuntu/pool/universe/n/ncurses/libtinfo5_6.3-2ubuntu0.2_amd64.deb
sudo apt install ./libtinfo5_6.3-2ubuntu0.2_amd64.deb

wget -qO- https://security.ubuntu.com/ubuntu/pool/universe/n/ncurses/ | grep -o 'libtinfo5_[^"]*amd64.deb'

sudo apt install locales
sudo locale-gen en_US.UTF-8
sudo update-locale LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8
```

## EBAZ4205用PetaLinuxプロジェクトの作成

```bash
petalinux-create project -n ebaz4205_linux --template zynq
cd ebaz4205_linux
petalinux-config --get-hw-description=../zynq7000_all_petalinux.xsa

petalinux-build -c device-tree -x do_configure
vi components/plnx_workspace/device-tree/device-tree/system-conf.dtsi

petalinux-build

cd images/linux/
petalinux-package boot --force --format BIN --fsbl zynq_fsbl.elf --fpga system.bit --u-boot
```

## SDカードへの書き込み

```bash
lsblk

SD=/dev/sdc
echo $SD

sudo umount ${SD}1 2>/dev/null
sudo umount ${SD}2 2>/dev/null

sudo wipefs -a $SD

sudo sfdisk $SD << 'EOF'
label: dos
unit: sectors

start=2048, size=204800, type=b
start=206848, type=83
EOF

sudo partprobe $SD
sleep 2
lsblk $SD

sudo mkfs.vfat -F 32 -n BOOT ${SD}1
sudo mkfs.ext4 -F -L rootfs ${SD}2

lsblk -f $SD

sudo cp images/linux/BOOT.BIN /media/user/BOOT/
sudo cp images/linux/image.ub /media/user/BOOT/
sudo cp images/linux/boot.scr /media/user/BOOT/

sudo tar -xzf images/linux/rootfs.tar.gz -C /media/user/rootfs/
```

## 開発ツールの組み込み

### Ubuntu PCのコマンド

```bash
cd ~/petalinux2024.2/ebaz4205_linux
petalinux-config -c rootfs

bootgen -image bootgen.bif -arch zynq -process_bitstream bin -w on

scp design_1_wrapper.bit.bin root@<EBAZ4205のIPアドレス>:/home/root/
```

### EBAZ4205のコマンド

```bash
ls /dev/tty*

ls -l /sys/class/gpio/

echo 512 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio512/direction
echo 1 > /sys/class/gpio/gpio512/value
echo 0 > /sys/class/gpio/gpio512/value
echo 512 > /sys/class/gpio/unexport

ifconfig

ls -l /home/root/design_1_wrapper.bit.bin

mkdir -p /lib/firmware/
cp /home/root/design_1_wrapper.bit.bin /lib/firmware/
echo 0 > /sys/class/fpga_manager/fpga0/flags
echo design_1_wrapper.bit.bin > /sys/class/fpga_manager/fpga0/firmware
cat /sys/class/fpga_manager/fpga0/state
```


