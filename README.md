# Secure Flash
Extract public keys & Signatures from key store & Capsule Header from input bios rom image. <br>
Verify public key, and signatures with input bios rom image. <br>
(Follow AMI's Secure Flash specification) <br>

## Buid
(Way 1) <br>
Step 1. cd into "json-c" folder and execute "make" <br>
Step 2. cd into "Common" folder and execute "make" <br>
Step 3. cd into "GetKey" folder and execute "make" <br>

<br>

(Way 2)<br>
Execute script in "Script" file.

## How to use
After the module being builded, cd into "bin" folder and construct the command below:
```
1. Help Menu.
   ./GetKey -h
   
2. Get public keys and signatures in capsule header from the input bios rom image.
   ./GetKey -gkc  <BIOS Image> -save(option, for saving)
   
3. Get public keys stored in keystore from the input bios rom image.
   ./GetKey -gks  <BIOS Image> -save(option, for saving)
   
4. Get rom layout in capsule header from the input bios rom image.
   ./GetKey -grl  <BIOS Image> -save(option, for saving)
```
