


sudo apt install cmake gcov-8 lcov

if [ "$TARGET_CPU" == "x86" ]; then
    sudo dpkg --add-architecture i386
    sudo apt-get -qq update

    # g++-multilib ставим в самом конце, после i386-пакетов!
    sudo apt-get install -y g++-8-multilib

else
    sudo apt install g++-8 gcc-8

fi


pip install conan