mkdir objdir
cd objdir
export CXX=clang++-10
../configure
make -j16
make check -j16
make install -j16
cd ..
rm -rf objdir
make clean
