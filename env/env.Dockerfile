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
    ln -s /usr/bin/wasm-ld-$LLVM_VERSION /usr/bin/wasm-ld                                                       &&\
    rm llvm.sh                                                                                                  &&\
    git clone --progress https://github.com/gittup/tup                                                          &&\
    cd tup                                                                                                      &&\
    git checkout e0e48ef68174e5078a6ad00f15ef91813f7ecc72                                                       &&\
    ./bootstrap-nofuse.sh                                                                                       &&\
    mv tup /usr/bin                                                                                             &&\
    cd ..                                                                                                       &&\
    rm -r tup                                                                                                   &&\
    git clone --progress --depth 1 https://github.com/ymattw/ydiff.git                                          &&\
    cd ydiff                                                                                                    &&\
    git checkout b6662c7646763151a52b67bc126fcd0979eaf5b3                                                       &&\
    /usr/bin/python2 setup.py install                                                                           &&\
    cd ..                                                                                                       &&\
    rm -r ydiff                                                                                                 &&\
    curl https://codeload.github.com/doctest/doctest/tar.gz/refs/tags/v2.4.9 > doctest-2.4.9.tar.gz             &&\
    tar xvf doctest-2.4.9.tar.gz                                                                                &&\
    cd doctest-2.4.9                                                                                            &&\
    clang++-${LLVM_VERSION} $FLAGS_DOCTEST -c doctest/parts/doctest.cpp -o libdoctest.a                         &&\
    mv libdoctest.a /usr/lib                                                                                    &&\
    cd ..                                                                                                       &&\
    rm -r doctest-2.4.9.tar.gz doctest-2.4.9                                                                    &&\
    curl https://www.sqlite.org/2022/sqlite-autoconf-3390200.tar.gz > sqlite-autoconf-3390200.tar.gz            &&\
    tar xvf sqlite-autoconf-3390200.tar.gz                                                                      &&\
    cd sqlite-autoconf-3390200                                                                                  &&\
    clang-${LLVM_VERSION} $FLAGS_SQLITE3 -c sqlite3.c -o libsqlite3.a                                           &&\
    mv libsqlite3.a /usr/lib                                                                                    &&\
    cd ..                                                                                                       &&\
    rm -r sqlite-autoconf-3390200 sqlite-autoconf-3390200.tar.gz                                                &&\
    git clone --progress --depth 1 https://github.com/nanomsg/nng.git                                           &&\
    cd nng                                                                                                      &&\
    git checkout 5385b788d28f42078b7fd342ab241e2043e158f5                                                       &&\
    cmake -G Ninja                                                                                              &&\
    ninja                                                                                                       &&\
    mv libnng.a /usr/lib                                                                                        &&\
    mv ./src/tools/nngcat/nngcat /usr/bin                                                                       &&\
    cd ..                                                                                                       &&\
    rm -rf nng                                                                                                  &&\
    git clone --progress --depth 1 https://github.com/camgunz/cmp.git                                           &&\
    cd cmp                                                                                                      &&\
    git checkout e836703291392aba9db92b46fb47929521fac71f                                                       &&\
    clang-${LLVM_VERSION} $FLAGS_CMP -c cmp.c -o libcmp.a                                                       &&\
    mv libcmp.a /usr/lib                                                                                        &&\
    cd ..                                                                                                       &&\
    rm -r cmp                                                                                                   &&\
    git clone --recursive --progress --depth 1 https://github.com/WebAssembly/wabt                              &&\
    cd wabt                                                                                                     &&\
    git checkout ed0b720c97ec6cc70b1dc33151b5dcadf12c31f4                                                       &&\
    mkdir build                                                                                                 &&\
    cd build                                                                                                    &&\
    cmake .. -G Ninja -DBUILD_TESTS=OFF                                                                         &&\
    ninja                                                                                                       &&\
    find -maxdepth 1 -executable -type f -exec mv -t /usr/bin {} +                                              &&\
    cd ../..                                                                                                    &&\
    rm -r wabt                                                                                                  &&\
    git clone --progress --depth 1 https://github.com/wasm3/wasm3.git                                           &&\
    cd wasm3                                                                                                    &&\
    git checkout 045040a97345e636b8be4f3086e6db59cdcc785f                                                       &&\
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
