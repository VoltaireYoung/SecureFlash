# Gey Key
Extract keys from key store in bios rom image.

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
	./GetKey <Bios rom image>
```
The keys would be saved into "Keys.bin".
