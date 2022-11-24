cd ./json-c
make clean
make

cd ..
cd ./Common
make clean
make

cd ..
cd ./Crypto
make clean
make

cd ..
cd ./GetKey
make clean
make
gcc -c -I .. -I ../Include/Common -I ../Include/ -I ../Include/IndustryStandard -I ../Common/ -I .. -I . -I ../Include/Ia32/ -I ../Crypto/  GetKey.c -o GetKey.o
gcc -o ../bin/GetKey GetKey.o Sdk/C/Alloc.o Sdk/C/LzFind.o Sdk/C/LzmaDec.o Sdk/C/LzmaEnc.o Sdk/C/7zFile.o Sdk/C/7zStream.o Sdk/C/Bra86.o -L../libs -lCommon -L../libs -ljsonc -L../libs -lCrypto
