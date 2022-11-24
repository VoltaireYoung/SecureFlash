cd bin

rm -f Cert_Header_Root.bin
rm -f Cert_Header_Sign.bin
rm -f PublicKey_Header_Root.bin
rm -f PublicKey_Header_Sign.bin
rm -f Rom_for_Verify.bin

read -p "Please input BIOS Image : " BIOS

./GetKey -gkc $BIOS -save
./GetKey -grl $BIOS -save

openssl dgst -sha256 -verify PublicKey_Header_Sign.bin -signature Cert_Header_Sign.bin Rom_for_Verify.bin

rm -f Cert_Header_Root.bin
rm -f Cert_Header_Sign.bin
rm -f PublicKey_Header_Root.bin
rm -f PublicKey_Header_Sign.bin
rm -f Rom_for_Verify.bin