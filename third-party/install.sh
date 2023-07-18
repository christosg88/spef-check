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

echo "Cleaning up build and src directories"
rm -rf pegtl-src pegtl-build fmt-src fmt-build
