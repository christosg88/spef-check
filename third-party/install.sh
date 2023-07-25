echo "Cloning pegtl-3.2.7"
git clone --depth 1 --branch 3.2.7 https://github.com/taocpp/PEGTL.git pegtl-src
cmake -B pegtl-build -S pegtl-src -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./pegtl-3.2.7
cmake --build pegtl-build -- -j$(nproc)
cmake --install pegtl-build

echo "Cloning fmt-10.0.0"
git clone --depth 1 --branch 10.0.0 https://github.com/fmtlib/fmt.git fmt-src
cmake -B fmt-build -S fmt-src -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./fmt-10.0.0
cmake --build fmt-build -- -j$(nproc)
cmake --install fmt-build

echo "Cloning zlib-1.2.13"
git clone --depth 1 --branch v1.2.13 https://github.com/madler/zlib.git zlib-src
cmake -B zlib-build -S zlib-src -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./zlib-1.2.13
cmake --build zlib-build -- -j$(nproc)
cmake --install zlib-build

echo "Cloning thread-pool"
git clone --depth 1 --branch v3.5.0 https://github.com/bshoshany/thread-pool.git thread-pool-3.5.0

echo "Cloning cpp_dbg_out"
git clone --depth 1 https://github.com/christosg88/cpp_dbg_out.git cpp_dbg_out

echo "Cleaning up build and src directories"
rm -rf pegtl-src pegtl-build fmt-src fmt-build zlib-src zlib-build
