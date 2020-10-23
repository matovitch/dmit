FROM ubuntu:20.04

ENV TO_INSTALL "               \
    libtinfo6                  \
    libncurses6                \
    pkg-config                 \
    software-properties-common \
    git-core                   \
    curl                       \
    fuse                       \
    libfuse-dev                \
    wget                       \
    jq                         \
    python2-minimal            \
    valgrind                   \
    libpcre3-dev               \
    cmake                      \
"

ENV TO_REMOVE "                \
    pkg-config                 \
    software-properties-common \
    curl                       \
    git-core                   \
    wget                       \
    libpcre3-dev               \
    libfuse-dev                \
    cmake                      \
"

ENV GUEST_GID "1000"
ENV GUEST_UID "1000"

ENV DEBIAN_FRONTEND "noninteractive"

ENV LLVM_VERSION "11"

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
    rm llvm.sh                                                                                                  &&\
    git clone --progress --depth 1 --branch v0.7.9 git://github.com/gittup/tup.git                              &&\
    cd tup                                                                                                      &&\
    ./bootstrap-nofuse.sh                                                                                       &&\
    mv tup /usr/bin                                                                                             &&\
    cd ..                                                                                                       &&\
    rm -r tup                                                                                                   &&\
    git clone --progress --depth 1 --branch 1.2 https://github.com/ymattw/ydiff.git                             &&\
    cd ydiff                                                                                                    &&\
    /usr/bin/python2 setup.py install                                                                           &&\
    cd ..                                                                                                       &&\
    rm -r ydiff                                                                                                 &&\
    curl https://codeload.github.com/onqtam/doctest/tar.gz/2.4.0 > doctest-2.4.0.tar.gz                         &&\
    tar xvf doctest-2.4.0.tar.gz                                                                                &&\
    cd doctest-2.4.0                                                                                            &&\
    clang++-${LLVM_VERSION} $FLAGS_DOCTEST -c doctest/parts/doctest.cpp -o libdoctest.a                         &&\
    mv libdoctest.a /usr/lib                                                                                    &&\
    cd ..                                                                                                       &&\
    rm -r doctest-2.4.0.tar.gz doctest-2.4.0                                                                    &&\
    curl https://www.sqlite.org/2020/sqlite-autoconf-3330000.tar.gz > sqlite-autoconf-3330000.tar.gz            &&\
    tar xvf sqlite-autoconf-3330000.tar.gz                                                                      &&\
    cd sqlite-autoconf-3330000                                                                                  &&\
    clang-${LLVM_VERSION} $FLAGS_SQLITE3 -c sqlite3.c -o libsqlite3.a                                           &&\
    mv libsqlite3.a /usr/lib                                                                                    &&\
    cd ..                                                                                                       &&\
    rm -r sqlite-autoconf-3330000 sqlite-autoconf-3330000.tar.gz                                                &&\
    git clone --progress --depth 1 --branch v1.3.2 https://github.com/nanomsg/nng.git                           &&\
    cd nng                                                                                                      &&\
    cmake -G "Unix Makefiles"                                                                                   &&\
    make                                                                                                        &&\
    mv libnng.a /usr/lib                                                                                        &&\
    cd ..                                                                                                       &&\
    rm -rf nng                                                                                                  &&\
    git clone --progress --depth 1 --branch v19 https://github.com/camgunz/cmp.git                              &&\
    cd cmp                                                                                                      &&\
    clang-${LLVM_VERSION} $FLAGS_CMP -c cmp.c -o libcmp.a                                                       &&\
    mv libcmp.a /usr/lib                                                                                        &&\
    cd ..                                                                                                       &&\
    rm -r cmp                                                                                                   &&\
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
