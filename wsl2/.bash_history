sudo dpkg --add-architecture i386
sudo apt -y install gawk wget git-core diffstat unzip texinfo gcc-multilib build-essential chrpath socat cpio python python3 python3-pip python3-pexpect xz-utils debianutils iputils-ping libsdl1.2-dev xterm autoconf automake libtool libglib2.0-dev libarchive-dev python-git sed cvs subversion mercurial coreutils texi2html docbook-utils python-pysqlite2 help2man make gcc g++ desktop-file-utils libgl1-mesa-dev libglu1-mesa-dev groff curl lzop asciidoc u-boot-tools dos2unix mtd-utils pv libelf-dev zlib1g-dev zlib1g:i386 libncurses5 libncurses5-dev libncursesw5-dev libgtk2.0-dev mc ffmpeg vlc imagemagick qemu-user-static
sudo update-binfmts --install i386 /usr/bin/qemu-i386-static --magic '\x7fELF\x01\x01\x01\x03\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x03\x00\x01\x00\x00\x00' --mask '\xff\xff\xff\xff\xff\xff\xff\xfc\xff\xff\xff\xff\xff\xff\xff\xff\xf8\xff\xff\xff\xff\xff\xff\xff'
sudo apt -y autoremove && sudo apt clean && sudo apt update && sudo apt -y upgrade
sudo service binfmt-support start
sudo service ssh start
mc ~/ /mnt/
export PATH=`pwd`:$PATH
arm-wrapper-linux-gnueabi-g++ snchot.cpp UdpSocket.cpp Thread.cpp -lpthread -o snchat.bin
sudo service binfmt-support start
for i in S s h hpp c cpp; do find ./ -name "*.$i" -exec sed -i 's/[ \t]*$//g' {} +; done
make clean &> /dev/null; make -j7 gcc8 &> /dev/null
git clone ssh://git@github.com/sentischev/myfiles.git
