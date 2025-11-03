


# Local build

Build on Linux/MacOS

```shell
brew install automake cmake 
mkdir -p cmake-build && cd cmake-build 
cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/usr -DCMAKE_BUILD_TYPE=Release  ..
cmake --build .

```

# Installer Build

Build on MUSL / alpine

```shell
apk add cmake gcc g++ patch make autoconf automake python3-dev py3-build py3-pip
mkdir -p cmake-build && cd cmake-build 
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release  ..
cmake --build .
```



