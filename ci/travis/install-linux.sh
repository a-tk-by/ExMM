
lsb_release -a
sudo apt-get update
sudo apt install cmake  lcov

if [ "$TARGET_CPU" == "x86" ]; then
    sudo dpkg --add-architecture i386
    sudo apt-get update

    # g++-multilib ставим в самом конце, после i386-пакетов!
    sudo apt-get install -y g++-8-multilib

else
    sudo apt install g++-8 gcc-8

fi


sudo pip install conan