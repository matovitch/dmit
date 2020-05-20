FROM ubuntu:latest

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
"

ENV TO_REMOVE "                \
    pkg-config                 \
    software-properties-common \
    curl                       \
    libfuse-dev                \
    git-core                   \
    wget                       \
    libpcre3-dev               \
"

ENV GUEST_GID "1000"
ENV GUEST_UID "1000"

ENV DEBIAN_FRONTEND "noninteractive"

COPY entrypoint.sh /

RUN set -ex                                                                                                     &&\
    apt-get update                                                                                              &&\
    apt-get install -y $TO_INSTALL                                                                              &&\
    apt-get update                                                                                              &&\
    llvm_version=10                                                                                             &&\
    curl https://apt.llvm.org/llvm.sh > llvm.sh                                                                 &&\
    chmod +x llvm.sh                                                                                            &&\
    ./llvm.sh $llvm_version                                                                                     &&\
    rm llvm.sh                                                                                                  &&\
    trim_length=$(($(echo $llvm_version | wc -c) + 1))                                                          &&\
    ls /usr/bin | tr ' ' '\n' | grep $llvm_version | while read bin;                                              \
                                                     do                                                           \
                                                         link=$(echo $bin | rev | cut -c ${trim_length}- | rev) &&\
                                                         ln -s /usr/bin/$bin                                      \
                                                               /usr/bin/$link;                                    \
                                                     done                                                       &&\
    git clone git://github.com/gittup/tup.git --progress                                                        &&\
    ln -s /usr/bin/clang /usr/bin/gcc                                                                           &&\
    cd tup                                                                                                      &&\
    ./bootstrap-nofuse.sh                                                                                       &&\
    mv tup /usr/bin                                                                                             &&\
    cd ..                                                                                                       &&\
    rm -r tup                                                                                                   &&\
    git clone https://github.com/ymattw/ydiff.git --progress                                                    &&\
    ln -s /usr/bin/python2 /usr/bin/python                                                                      &&\
    cd ydiff                                                                                                    &&\
    ./setup.py install                                                                                          &&\
    cd ..                                                                                                       &&\
    rm -r ydiff                                                                                                 &&\
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
