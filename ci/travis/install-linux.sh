
lsb_release -a
sudo apt-get update
sudo apt install cmake lcov

if [ "$TARGET_CPU" == "x86" ]; then
    sudo dpkg --add-architecture i386
    sudo apt-get update
    # g++-multilib ставим в самом конце, после i386-пакетов!
    sudo apt-get install -y g++-multilib gcc-multilib linux-libc-dev:i386

fi
