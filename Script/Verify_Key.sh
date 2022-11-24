cd bin

rm -f Cert_Header_Root.bin
rm -f Cert_Header_Sign.bin
rm -f PublicKey_Header_Root.bin
rm -f PublicKey_Header_Sign.bin
rm -f PublicKey_KeyStore.bin

read -p "Please input first  BIOS Image : " BIOS1
read -p "Please input second BIOS Image : " BIOS2

./GetKey -gkc $BIOS1 -save
./GetKey -gks $BIOS2 -save

./GetKey -vk PublicKey_KeyStore.bin PublicKey_Header_Sign.bin

rm -f Cert_Header_Root.bin
rm -f Cert_Header_Sign.bin
rm -f PublicKey_Header_Root.bin
rm -f PublicKey_Header_Sign.bin
rm -f PublicKey_KeyStore.bin
