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
2. Get public keys and signatures from capsule header.
   ./GetKey -gkc 
./GetKey -gkc <BIOS Image> -save(Option)
./GetKey -grl <BIOS Image> -save(Option)
```
