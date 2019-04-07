FROM ubuntu:devel

ENV TO_INSTALL "               \
    libtinfo6                  \
    libncurses6                \
    pkg-config                 \
    software-properties-common \
    git-core                   \
    curl                       \
    fuse                       \
    libfuse-dev                \
"

ENV TO_REMOVE "                \
    pkg-config                 \
    software-properties-common \
    curl                       \
    libfuse-dev                \
    git-core                   \
"

ENV GUEST_GID "1000"
ENV GUEST_UID "1000"

COPY entrypoint.sh /

RUN set -ex                                                                                                     &&\
    apt-get update                                                                                              &&\
    apt-get install -y $TO_INSTALL                                                                              &&\
    apt-get update                                                                                              &&\
    curl http://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -                                              &&\
    llvm_version=$(                                                                                               \
        curl http://apt.llvm.org/unstable/dists/ 2> /dev/null | grep llvm                                         \
                                                              | tail -n 1                                         \
                                                              | grep -Eo 'llvm-toolchain-[0-9.]*'                 \
                                                              | head -n 1                                         \
                                                              | rev                                               \
                                                              | cut -d '-' -f 1                                   \
                                                              | rev                                           ) &&\
    apt-add-repository "deb http://apt.llvm.org/unstable/ llvm-toolchain-$llvm_version main"                    &&\
    apt-get install -y                                                                                            \
        libllvm$llvm_version                                                                                      \
        llvm-$llvm_version                                                                                        \
        llvm-$llvm_version-dev                                                                                    \
        llvm-$llvm_version-doc                                                                                    \
        llvm-$llvm_version-examples                                                                               \
        llvm-$llvm_version-runtime                                                                                \
        clang-$llvm_version                                                                                       \
        clang-tools-$llvm_version                                                                                 \
        clang-$llvm_version-doc                                                                                   \
        libclang-common-$llvm_version-dev                                                                         \
        libclang-$llvm_version-dev                                                                                \
        libclang1-$llvm_version                                                                                   \
        liblldb-$llvm_version                                                                                     \
        lldb-$llvm_version                                                                                        \
        lld-$llvm_version                                                                                         \
        libc++-$llvm_version-dev                                                                                  \
        libc++abi-$llvm_version-dev                                                                             &&\
    trim_length=$(($(echo $llvm_version | wc -c) + 1))                                                          &&\
    ls /usr/bin | tr ' ' '\n' | grep $llvm_version | while read bin;                                              \
                                                     do                                                           \
                                                         link=$(echo $bin | rev | cut -c ${trim_length}- | rev) &&\
                                                         ln -s /usr/bin/$bin                                      \
                                                               /usr/bin/$link;                                    \
                                                     done                                                       &&\
    git clone git://github.com/gittup/tup.git --progress                                                        &&\
    cd tup                                                                                                      &&\
    ./bootstrap-nofuse.sh                                                                                       &&\
    mv tup /usr/bin                                                                                             &&\
    cd ..                                                                                                       &&\
    rm -r tup                                                                                                   &&\
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
