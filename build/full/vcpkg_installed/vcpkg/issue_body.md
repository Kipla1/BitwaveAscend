Package: libsodium:x64-osx@1.0.22#1

**Host Environment**

- Host: x64-osx
- Compiler: AppleClang 14.0.0.14000029
- CMake Version: 4.4.3
-    vcpkg-tool version: 2026-07-27-98d7cb0cf1f4686a3e43aa5672b6230c1d56bce8
    vcpkg-scripts version: 07f4812200 2026-09-27 (12 hours ago)

**To Reproduce**

`vcpkg install `

**Failure logs**

```
Downloading https://github.com/jedisct1/libsodium/archive/1.0.22-RELEASE.tar.gz -> jedisct1-libsodium-1.0.22-RELEASE.tar.gz
Successfully downloaded jedisct1-libsodium-1.0.22-RELEASE.tar.gz
-- Extracting source /Users/Oscar/Documents/school/vcpkg/downloads/jedisct1-libsodium-1.0.22-RELEASE.tar.gz
-- Applying patch 001-mingw-i386.patch
-- Using source at /Users/Oscar/Documents/school/vcpkg/buildtrees/libsodium/src/22-RELEASE-59fa844d12.clean
-- Getting CMake variables for x64-osx
-- Loading CMake variables from /Users/Oscar/Documents/school/vcpkg/buildtrees/libsodium/cmake-get-vars_C_CXX-x64-osx.cmake.log
CMake Error at /Users/Oscar/Documents/school/BitwaveAscend/build/full/vcpkg_installed/x64-osx/share/vcpkg-make/vcpkg_make.cmake:108 (message):
  libsodium currently requires the following programs from the system package
  manager:

      autoconf autoconf-archive automake libtoolize



      On Debian and Ubuntu derivatives:
          sudo apt install autoconf autoconf-archive automake libtool
      On recent Red Hat and Fedora derivatives:
          sudo dnf install autoconf autoconf-archive automake libtool
      On Arch Linux and derivatives:
          sudo pacman -S autoconf autoconf-archive automake libtool
      On Alpine:
          apk add autoconf autoconf-archive automake libtool
      On macOS:
          brew install autoconf autoconf-archive automake libtool

Call Stack (most recent call first):
  /Users/Oscar/Documents/school/BitwaveAscend/build/full/vcpkg_installed/x64-osx/share/vcpkg-make/vcpkg_make_configure.cmake:66 (vcpkg_run_autoreconf)
  buildtrees/versioning_/versions/libsodium/95eb2b61a8632cbfe65fd3cf259f8805b8792364/portfile.cmake:85 (vcpkg_make_configure)
  scripts/ports.cmake:209 (include)



```

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "name": "bitwave-ascend",
  "version": "0.1.0",
  "description": "University 2D game project (OOP) - Bitwave Ascend",
  "builtin-baseline": "07f4812200df3d3c931c0c8a6081d3b21fe2bf9f",
  "dependencies": [
    "sfml",
    "sqlite3",
    "libsodium",
    "nlohmann-json",
    "spdlog",
    {
      "name": "catch2",
      "features": []
    }
  ]
}

```
</details>
