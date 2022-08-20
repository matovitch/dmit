FROM ubuntu:22.04

ENV TO_INSTALL "               \
    libtinfo6                  \
    libncurses6                \
    pkg-config                 \
    software-properties-common \
    git-core                   \
    curl                       \
    libfuse3-dev               \
    fuse3                      \
    wget                       \
    jq                         \
    python2-minimal            \
    valgrind                   \
    tmux                       \
    libpcre3-dev               \
    ninja-build                \
    cmake                      \
"

ENV TO_REMOVE "                \
    pkg-config                 \
    software-properties-common \
    curl                       \
    git-core                   \
    wget                       \
    libpcre3-dev               \
    libfuse3-dev               \
    cmake                      \
    ninja-build                \
"

ENV GUEST_GID "1000"
ENV GUEST_UID "1000"

ENV DEBIAN_FRONTEND "noninteractive"

ENV LLVM_VERSION "16"

ENV FLAGS_DOCTEST "-D DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN"
ENV FLAGS_SQLITE3 "-D SQLITE_ENABLE_DESERIALIZE"
ENV FLAGS_CMP     ""

ENV CXX "/usr/bin/clang++-${LLVM_VERSION}"

COPY entrypoint.sh /

RUN set -ex                                                                                                     &&\
    apt-get update                                                                                              &&\
    apt-get install -y $TO_INSTALL                                                                              &&\
    apt-get update                                                                                              &&\
    curl https://apt.llvm.org/llvm.sh > llvm.sh                                                                 &&\
    chmod +x llvm.sh                                                                                            &&\
    ./llvm.sh $LLVM_VERSION                                                                                     &&\
    ln -s /usr/bin/clang++-$LLVM_VERSION /usr/bin/clang++                                                       &&\
    ln -s /usr/bin/ld.lld-$LLVM_VERSION /usr/bin/ld.lld                                                         &&\
    ln -s /usr/bin/lld-$LLVM_VERSION /usr/bin/lld                                                               &&\
    rm llvm.sh                                                                                                  &&\
    git clone --progress --depth 1 https://github.com/gittup/tup                                                &&\
    cd tup                                                                                                      &&\
    git checkout 7149db11be1728580f1ddcf760208d2ca85e7558                                                       &&\
    ./bootstrap-nofuse.sh                                                                                       &&\
    mv tup /usr/bin                                                                                             &&\
    cd ..                                                                                                       &&\
    rm -r tup                                                                                                   &&\
    git clone --progress --depth 1 --branch 1.2 https://github.com/ymattw/ydiff.git                             &&\
    cd ydiff                                                                                                    &&\
    /usr/bin/python2 setup.py install                                                                           &&\
    cd ..                                                                                                       &&\
    rm -r ydiff                                                                                                 &&\
    curl https://codeload.github.com/onqtam/doctest/tar.gz/2.4.6 > doctest-2.4.6.tar.gz                         &&\
    tar xvf doctest-2.4.6.tar.gz                                                                                &&\
    cd doctest-2.4.6                                                                                            &&\
    clang++-${LLVM_VERSION} $FLAGS_DOCTEST -c doctest/parts/doctest.cpp -o libdoctest.a                         &&\
    mv libdoctest.a /usr/lib                                                                                    &&\
    cd ..                                                                                                       &&\
    rm -r doctest-2.4.6.tar.gz doctest-2.4.6                                                                    &&\
    curl https://www.sqlite.org/2021/sqlite-autoconf-3360000.tar.gz > sqlite-autoconf-3360000.tar.gz            &&\
    tar xvf sqlite-autoconf-3360000.tar.gz                                                                      &&\
    cd sqlite-autoconf-3360000                                                                                  &&\
    clang-${LLVM_VERSION} $FLAGS_SQLITE3 -c sqlite3.c -o libsqlite3.a                                           &&\
    mv libsqlite3.a /usr/lib                                                                                    &&\
    cd ..                                                                                                       &&\
    rm -r sqlite-autoconf-3360000 sqlite-autoconf-3360000.tar.gz                                                &&\
    git clone --progress --depth 1 --branch v1.5.2 https://github.com/nanomsg/nng.git                           &&\
    cd nng                                                                                                      &&\
    cmake -G Ninja                                                                                              &&\
    ninja                                                                                                       &&\
    mv libnng.a /usr/lib                                                                                        &&\
    mv ./src/tools/nngcat/nngcat /usr/bin                                                                       &&\
    cd ..                                                                                                       &&\
    rm -rf nng                                                                                                  &&\
    git clone --progress --depth 1 --branch v19 https://github.com/camgunz/cmp.git                              &&\
    cd cmp                                                                                                      &&\
    clang-${LLVM_VERSION} $FLAGS_CMP -c cmp.c -o libcmp.a                                                       &&\
    mv libcmp.a /usr/lib                                                                                        &&\
    cd ..                                                                                                       &&\
    rm -r cmp                                                                                                   &&\
    git clone --recursive --progress --depth 1 https://github.com/matovitch/wasp.git                            &&\
    cd wasp                                                                                                     &&\
    git checkout 18ca152d274f1056fccf5ea0fe93794f0058c17e                                                       &&\
    cmake -G Ninja -DBUILD_TESTING=OFF                                                                          &&\
    ninja                                                                                                       &&\
    mv src/tools/wasp /usr/bin                                                                                  &&\
    cd ..                                                                                                       &&\
    rm -r wasp                                                                                                  &&\
    git clone --progress --depth 1 https://github.com/wasm3/wasm3.git                                           &&\
    cd wasm3                                                                                                    &&\
    git checkout 9dcfce271c2fac86823725fc9ec0f75309d820e4                                                       &&\
    cmake -G Ninja                                                                                              &&\
    ninja                                                                                                       &&\
    mv source/libm3.a /usr/lib                                                                                  &&\
    cd ..                                                                                                       &&\
    rm -r wasm3                                                                                                 &&\
    apt-get remove -y $TO_REMOVE                                                                                &&\
    apt-get autoremove -y                                                                                       &&\
    apt-get clean                                                                                               &&\
    groupadd --gid $GUEST_GID guest                                                                             &&\
    useradd                                                                                                       \
        --uid $GUEST_UID                                                                                          \
        --gid $GUEST_GID                                                                                          \
        --shell /bin/bash                                                                                         \
    guest                                                                                                       &&\
    chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
