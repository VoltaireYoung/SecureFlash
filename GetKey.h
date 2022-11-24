#include "json-c/json.h"
#include "Crypto/rsa256.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/des.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

#include <FvLib.h>
#include <Common/UefiBaseTypes.h>
#include <Common/UefiCapsule.h>
#include <Common/PiFirmwareFile.h>
#include <Common/PiFirmwareVolume.h>
#include <Guid/PiFirmwareFileSystem.h>
#include <IndustryStandard/PeImage.h>
#include <Protocol/GuidedSectionExtraction.h>

#include "Compress.h"
#include "Decompress.h"
#include "CommonLib.h"
#include "EfiUtilityMsgs.h"
#include "FirmwareVolumeBufferLib.h"
#include "OsPath.h"
#include "ParseGuidedSectionTools.h"
#include "StringFuncs.h"
#include "ParseInf.h"
#include "PeCoffLib.h"

#include "Sdk/C/LzmaDec.h"
#include "Sdk/C/Alloc.h"
#include <Common/UefiInternalFormRepresentation.h>
#include <Common/Nvram.h>

//#include <Common/UefiBaseTypes.h>
#include <Common/UefiMultiPhase.h>
//#include <Common/UefiCapsule.h>

// ---- Function Declaration ----

// Get Key ＆ Signature function
void Get_Keys_and_Signatures_from_Capsule_Header(char*,UINT8***,UINT8***);
void Get_Keys_from_Key_Store(char*,UINT8***,UINTN*);
void Get_Rom_for_Verify(char* ROM_Image_Name , UINTN Capsule_Header_Offset , UINT8*** Materials , UINTN** Materials_Size , UINT8* Number_of_Materials);

// Verify
BOOLEAN Verify_Key(UINT8* Key1 , UINTN Size_of_Key1 , UINT8* Key2 , UINTN Size_of_Key2);

// My Tools
UINTN Find_Aptio_FW_Capsule_Header_Offset(char* ROM_Image_Name);
UINTN Find_Firmware_Volume_Offset(char* ROM_Image_Name , EFI_GUID* Taraget_FV_GUID);
UINTN Find_FFS_Offset(char* ROM_Image_Name , UINTN* FV_Offset , EFI_GUID* Target_FFS_GUID);
UINTN Find_Section_Offset_by_SubTypeGUID(char* ROM_Image_Name , UINTN* FFS_Offset , EFI_GUID* Target_Section_GUID);

void Save_Key(UINT8*** Keys , UINTN index_of_keys , char* path , UINTN Size);
void Save_Signature(UINT8*** Signatures , UINTN index_of_keys , char* path , UINTN Size);

BOOLEAN Check_FV_Signature(UINT32*);
int Get_Number_of_Key(EFI_FFS_FILE_HEADER* input);
UINTN Get_Length(UINT8*);

// For chekinng
void Print_FV_Header(EFI_FIRMWARE_VOLUME_HEADER*);
void Print_GUID(EFI_GUID*);
void Print_Signature(UINT32*);
void Print_Attribute(EFI_FVB_ATTRIBUTES_2*);
void Print(UINT8* array , UINTN size);
void Print_Rom_Layout(char* ROM_Image_Name , UINTN Capsule_Header_Offset , BOOLEAN Show_Material_for_Verify);

// UI
void My_UI(int argc , char* argv[]);
void UI_Help(char* instruction);
void UI_Get_Key_and_Signature_from_Capsule_Header(int argc , char* argv[]);
void UI_Get_Key_from_Key_Store(int argc , char* argv[]);
void UI_Get_Rom_Layout(int argc , char* argv[]);
// void UI_Verify(int argc , char* argv[]);

// ---- Parameters & Structures ----

#define FV_BB_GUID \
  { \
    0x61C0F511, 0xA691, 0x4F54, {0x97, 0x4F, 0xB9, 0xA4, 0x21, 0x72, 0xCE, 0x53 } \
  }

#define FV_MAIN_GUID \
  { \
    0x4F1C52D3, 0xD824, 0x4D2A, {0xA2, 0xF0, 0xEC, 0x40, 0xC2, 0x3C, 0x59, 0x16 } \
  }

#define CAPSULE_HEADER_GUID \
  { \
    0x414D94AD, 0x998D, 0x47D2, {0xBF, 0xCD, 0x4E, 0x88, 0x22, 0x41, 0xDE, 0x32 } \
  }

#define Key_Store_GUID \
  { \
    0x5B85965C, 0x455D, 0x4CC6, {0x9C, 0x4C, 0x7F, 0x08, 0x69, 0x67, 0xD2, 0xB0 } \
  }

#define SECURE_GUID \
  { \
    0x5A88641B, 0xBBB9, 0x4AA6, {0x80, 0xF7, 0x49, 0x8A, 0xE4, 0x07, 0xC3, 0x1F } \
  }

#define KEY_SIZE_IN_KEY_STORE 32
#define KEY_SIZE_IN_CAPSULE 256
#define SIGNATURE_SIZE_IN_CAPSULE 256

typedef struct{
  WIN_CERTIFICATE_UEFI_GUID Hdr;
  EFI_CERT_BLOCK_RSA_2048_SHA256 CertData;
}AMI_CERTIFICATE_RSA2048_SHA256;

typedef struct{
  AMI_CERTIFICATE_RSA2048_SHA256 SignCert;
  EFI_CERT_BLOCK_RSA_2048_SHA256 RootCert;
}FW_CERTIFICATE;

typedef enum {
    RomAreaTypeFv,
    RomAreaTypeRaw,
    RomAreaTypeMax
} ROM_AREA_TYPE;

typedef struct{
  EFI_PHYSICAL_ADDRESS Address;
  UINT32 Offset;
  UINT32 Size;
  ROM_AREA_TYPE Type;
  UINT32 Attributes;
}ROM_AREA;

typedef struct{
  EFI_CAPSULE_HEADER      CapHdr;
  UINT16                  RomImageOffset;
  UINT16                  RomLayoutOffset;
  FW_CERTIFICATE          FWCert;
  ROM_AREA                RomAreaMap[1];
}APTIO_FW_CAPSULE_HEADER;

EFI_GUID gEfiHashAlgorithmSha256Guid = { 0x51AA59DE, 0xFDF2, 0x4EA3, { 0xBC, 0x63, 0x87, 0x5F, 0xB7, 0x84, 0x2E, 0xE9 }};
EFI_GUID gEfiCertRsa2048Guid = { 0x3c5766e8, 0x269c, 0x4e34, {0xaa, 0x14, 0xed, 0x77, 0x6e, 0x85, 0xb3, 0xb6 }};

#define AMI_ROM_AREA_SIGNED 0x00000200
#define CAPSULE_FLAGS_CAPHDR_IN_SIGNCERT        0x00000002 
#define CAPSULE_FLAGS_CAPHDR_IN_SIGNCERT 0x00000002
#define DEFAULT_RSA_SIG_LEN 256
#define SHA256_DIGEST_SIZE 32

#define E_CONST	0x01, 0x00, 0x01

UINT8 Public_Key_Header[] = {0x30 , 0x82 , 0x01 , 0x0a , 0x02 , 0x82 , 0x01 , 0x01 , 0x00};
UINT8 Public_Key_Tail[] = {0x02 , 0x03 , 0x01 , 0x00 , 0x01};


// UI

#define Instruction_Help "-h"
#define Instruction_Get_Key_and_Signature_from_Capsule_Header "-gkc"
#define Instruction_Get_Key_from_Key_Store "-gks"
#define Instruction_Get_Rom_Layout "-grl"
#define Instruction_Verify_Key "-vk"

char* Instruction_Name[] = {
  "Help",
  "Get keys and signatures from capsule Header",
  "Get keys from key store",
  "Get rom layout",
  "Verify key"
};

char* Instructions[] = {
  "-h",
  "-gkc",
  "-gks",
  "-grl",
  "-vk"
};

char* Subinstructions[] = {
  "",
  "-save",
  "-save",
  "",
  ""
};

char* Instruction_Usage[] = {
  "",
  "<Bios Rom Image>  <-save>(Optional)",
  "<Bios Rom Image>  <-save>(Optional)",
  "<Bios Rom Image>",
  "<file 1 name>  <file 2 name>"
};

char* Instruction_Description[] = {

};