#include "GetKey.h"

// ---- Function Implementation ----

int main(int argc , char* argv[]){
  My_UI(argc , argv);

  return 0;
}

// Get Key ＆ Signature function
void Get_Keys_and_Signatures_from_Capsule_Header(char* ROM_Image_Name , UINT8*** Keys , UINT8*** Signatures){
  if(ROM_Image_Name == NULL){
    printf("In Get_Keys_and_Signatures_from_Capsule_Header\n");
    printf("Input Error : ROM_Image_Name\n");
    return;
  }

  // ---- Variables Declaration ----
  EFI_GUID GUID_FV_MAIN = FV_MAIN_GUID;
  EFI_GUID GUID_CAPSULE_HEADER = CAPSULE_HEADER_GUID;
  EFI_GUID GUID_SECURE = SECURE_GUID;

  UINTN FV_Offset = 0;
  UINTN FFS_Offset = 0;
  UINTN Section_Offset = 0;
  UINTN Capsule_Header_Offset = 0;

  APTIO_FW_CAPSULE_HEADER Capsule_Header;
  FILE* ROM_Image = NULL;

  int current_index;

  // ---- Step 1. Get Offset of Capsule Header ----
  FV_Offset = Find_Firmware_Volume_Offset(ROM_Image_Name , &GUID_FV_MAIN);
  FFS_Offset = Find_FFS_Offset(ROM_Image_Name , &FV_Offset , &GUID_CAPSULE_HEADER);
  Section_Offset = Find_Section_Offset_by_SubTypeGUID(ROM_Image_Name , &FFS_Offset , &GUID_SECURE);
  Capsule_Header_Offset = Section_Offset + sizeof(EFI_FREEFORM_SUBTYPE_GUID_SECTION);

  // ---- Step 2. Read Capsule Header ----
  ROM_Image = fopen(LongFilePath(ROM_Image_Name) , "rb");
  if(ROM_Image == NULL){
    printf("In Get_Keys_and_Signatures_from_Capsule_Header\n");
    printf("Open File Error\n");
    return;
  }

  fseek(ROM_Image , Capsule_Header_Offset , SEEK_SET);
  fread(&Capsule_Header , sizeof (APTIO_FW_CAPSULE_HEADER), 1, ROM_Image);

  // ---- Step 3. Extract Keys & Signatures ----
  *Keys = (UINT8**)malloc( 2 * sizeof(UINT8*));
  *Signatures = (UINT8**)malloc( 2 * sizeof(UINT8*));

  if(NULL == *Keys){
    printf("In Get_Keys_and_Signatures_from_Capsule_Header\n");
    printf("Memory Allocation Error : Keys\n");
    return;
  }
  else if(NULL == *Signatures){
    printf("In Get_Keys_and_Signatures_from_Capsule_Header\n");
    printf("Memory Allocation Error : Signatures\n");
    return;
  }

  for(current_index = 0 ; current_index < 2 ; current_index++){
    (*Keys)[current_index] = (UINT8*)malloc(KEY_SIZE_IN_CAPSULE * sizeof(UINT8));
    (*Signatures)[current_index] = (UINT8*)malloc(KEY_SIZE_IN_CAPSULE * sizeof(UINT8));

    if(NULL == (*Keys)[current_index]){
      printf("In Get_Keys_and_Signatures_from_Capsule_Header\n");
      printf("Memory Allocation Error : Keys[%d]\n" , current_index);
      return;
    }
    else if(NULL == (*Signatures)[current_index]){
      printf("In Get_Keys_and_Signatures_from_Capsule_Header\n");
      printf("Memory Allocation Error : Sigantures[%d]\n" , current_index);
      return;
    }
  }
  
  memcpy((*Keys)[0] , Capsule_Header.FWCert.SignCert.CertData.PublicKey , 256); // SignCert Public Key
  memcpy((*Keys)[1] , Capsule_Header.FWCert.RootCert.PublicKey , 256); // SignCert Public Key
  memcpy((*Signatures)[0] , Capsule_Header.FWCert.SignCert.CertData.Signature , 256); // SignCert Public Key
  memcpy((*Signatures)[1] , Capsule_Header.FWCert.RootCert.Signature , 256); // SignCert Public Key

  return 0;
}

void Get_Keys_from_Key_Store(char* ROM_Image_Name , UINT8*** Keys , UINTN* Number_of_Keys){
  if(ROM_Image_Name == NULL){
    printf("In Get_Keys_and_Signatures_from_Capsule_Header\n");
    printf("Input Error : ROM_Image_Name\n");
    return;
  }

  // ---- Variables Declaration ----
  EFI_GUID GUID_FV_BB = FV_BB_GUID;
  EFI_GUID GUID_Key_Store = Key_Store_GUID;

  UINTN FV_Offset = 0;
  UINTN FFS_Offset = 0;
  UINTN Current_Key_Offset;

  EFI_FFS_FILE_HEADER FFS_Header;
  FILE* ROM_Image = NULL;

  int number_of_keys = 0;
  UINT8 Current_Key[KEY_SIZE_IN_KEY_STORE];

  int current_index;

  // ---- Step 1. Get Offset of Capsule Header ----
  FV_Offset = Find_Firmware_Volume_Offset(ROM_Image_Name , &GUID_FV_BB);
  FFS_Offset = Find_FFS_Offset(ROM_Image_Name , &FV_Offset , &GUID_Key_Store);
  
  // ---- Step 2. Read Key Store----
  ROM_Image = fopen(LongFilePath(ROM_Image_Name) , "rb");
  if(ROM_Image == NULL){
    printf("In Get_Keys_from_Key_Store\n");
    printf("Open File Error\n");
    return;
  }

  fseek(ROM_Image , FFS_Offset , SEEK_SET);
  fread(&FFS_Header , sizeof (EFI_FFS_FILE_HEADER), 1, ROM_Image);

  // ---- Step 3. Extract Keys ----
  *Number_of_Keys = Get_Number_of_Key(&FFS_Header);
  
  (*Keys) = (UINT8**)malloc(number_of_keys * sizeof(UINT8*));

  if(NULL == *Keys){
    printf("In Get_Keys_from_Key_Store\n");
    printf("Memory Allocation Error : Keys\n");
    return;
  }

  for(current_index = 0 ; current_index < *Number_of_Keys ; current_index++){
    (*Keys)[current_index] = (UINT8*)malloc(KEY_SIZE_IN_KEY_STORE * sizeof(UINT8));

    if(NULL == (*Keys)[current_index]){
      printf("In Get_Keys_from_Key_Store\n");
      printf("Memory Allocation Error : Keys[%d]\n" , current_index);
      return;
    }

    Current_Key_Offset = FFS_Offset + sizeof(EFI_FFS_FILE_HEADER) + 4 + (KEY_SIZE_IN_KEY_STORE * current_index);
    fseek(ROM_Image , Current_Key_Offset , SEEK_SET);
    fread(Current_Key , KEY_SIZE_IN_KEY_STORE , 1 , ROM_Image);

    memcpy((*Keys)[current_index] , Current_Key , KEY_SIZE_IN_KEY_STORE);
  }
}

void Get_Rom_for_Verify(char* ROM_Image_Name , UINTN Capsule_Header_Offset , UINT8*** Materials , UINTN** Materials_Size , UINT8* Number_of_Materials){
  // ---- Variables Declaration ----
  FILE* ROM_Image = NULL;
  
  APTIO_FW_CAPSULE_HEADER Capsule_Header;

  ROM_AREA* RomLayout_Table;
  UINTN Rom_Layout_Offset = 0;
  UINTN Rom_Layout_Size = 0;
  UINTN Rom_Layout_Max_Number = 0;
  UINTN Number_of_Material_from_Rom_Layout = 0;
  
  UINTN Current_Material_Offset;
  UINTN Current_Material_Size;

  int current_index;
  int current_material_index;

  // ---- Implementation ----
  ROM_Image = fopen(LongFilePath(ROM_Image_Name) , "rb");
  if(ROM_Image == NULL){
    printf("In Get_Hash_Material_According_to_Rom_Map\n");
    printf("Open File Error\n");
    return;
  }

  fseek(ROM_Image , Capsule_Header_Offset , SEEK_SET);
  fread(&Capsule_Header , sizeof (APTIO_FW_CAPSULE_HEADER), 1, ROM_Image);

  Rom_Layout_Offset = Capsule_Header_Offset + Capsule_Header.RomLayoutOffset;
  Rom_Layout_Size = Capsule_Header.RomImageOffset - Capsule_Header.RomLayoutOffset;
  Rom_Layout_Max_Number = Rom_Layout_Size / sizeof(ROM_AREA);
  RomLayout_Table = (ROM_AREA*)malloc(Rom_Layout_Max_Number * sizeof(ROM_AREA));
  fseek(ROM_Image , Rom_Layout_Offset , SEEK_SET);
  fread(RomLayout_Table , sizeof (ROM_AREA), Rom_Layout_Max_Number, ROM_Image);

  // Statistic Number of Materials
  for(current_index = 0 ; current_index < Rom_Layout_Max_Number ; current_index++){
    if(RomLayout_Table[current_index].Attributes & AMI_ROM_AREA_SIGNED){
      Number_of_Material_from_Rom_Layout++;
    }
  }

  (*Number_of_Materials) = Number_of_Material_from_Rom_Layout + 1; // At least Rom Map need to be hashed
  if(Capsule_Header.CapHdr.Flags & CAPSULE_FLAGS_CAPHDR_IN_SIGNCERT){
    (*Number_of_Materials) += 1;
  }

  // Extract Material
  *Materials = (UINT8**)malloc((*Number_of_Materials) * sizeof(UINT8*));
  *Materials_Size = (UINT8*)malloc((*Number_of_Materials) * sizeof(UINT8));
  if(*Materials == NULL){
    printf("In Get_Hash_Material_According_to_Rom_Map\n");
    printf("Materails Memory Allocation Error\n");
    return;
  }
  else if(*Materials_Size == NULL){
    printf("In Get_Hash_Material_According_to_Rom_Map\n");
    printf("Materails_Size Memory Allocation Error\n");
    return;
  }

  for(current_index = 0 , current_material_index = 0 ; current_index < Rom_Layout_Max_Number && RomLayout_Table[current_index].Size != 0 ; current_index++){
    if(RomLayout_Table[current_index].Attributes & AMI_ROM_AREA_SIGNED){
      Current_Material_Size = RomLayout_Table[current_index].Size;

      (*Materials)[current_material_index] = (UINT8*)malloc(Current_Material_Size * sizeof(UINT8));
      if((*Materials)[current_material_index] == NULL){
        printf("In Get_Hash_Material_According_to_Rom_Map\n");
        printf("Materals[current_index] Memory Allocation Error\n");
        return;
      }
      Current_Material_Offset = RomLayout_Table[current_index].Offset;
      fseek(ROM_Image , Current_Material_Offset , SEEK_SET);
      fread((*Materials)[current_material_index], sizeof(UINT8), Current_Material_Size, ROM_Image);
      
      (*Materials_Size)[current_material_index] = Current_Material_Size;

      current_material_index++;
    }
  }

  // Put Header as Material according to Flags
  if(Capsule_Header.CapHdr.Flags & CAPSULE_FLAGS_CAPHDR_IN_SIGNCERT){
    Current_Material_Offset = Capsule_Header_Offset;
    Current_Material_Size = (UINTN)&Capsule_Header.FWCert.SignCert.CertData - (UINTN)&Capsule_Header;
    (*Materials)[current_material_index] = (UINT8*)malloc(Current_Material_Size * sizeof(UINT8));
    fseek(ROM_Image , Current_Material_Offset , SEEK_SET);
    fread((*Materials)[current_material_index], sizeof(UINT8), Current_Material_Size, ROM_Image);

    (*Materials_Size)[current_material_index] = Current_Material_Size;

    current_material_index++;
  }

  // Put Rom Layout Map as Material
  Current_Material_Offset = Rom_Layout_Offset;
  //Current_Material_Offset = 0x60E50;
  Current_Material_Size = (current_index + 1) * sizeof(ROM_AREA);
  printf("Rom Layout Offset : %X\n" , Current_Material_Offset);
  printf("Rom Layout size : %X\n" , Current_Material_Size);
  (*Materials)[current_material_index] = (UINT8*)malloc(Current_Material_Size * sizeof(UINT8));
  fseek(ROM_Image , Current_Material_Offset , SEEK_SET);
  fread((*Materials)[current_material_index], sizeof(UINT8), Current_Material_Size, ROM_Image);
  (*Materials_Size)[current_material_index] = Current_Material_Size;
  printf("%X -- %X -- %X\n",Materials[0][current_material_index][0],Materials[0][current_material_index][1],Materials[0][current_material_index][2]);
}

// Verify
BOOLEAN Verify_Key(UINT8* Key1 , UINTN Size_of_Key1 , UINT8* Key2 , UINTN Size_of_Key2){
  if(Size_of_Key1 != KEY_SIZE_IN_KEY_STORE && Size_of_Key2 != KEY_SIZE_IN_KEY_STORE){
    printf("In Verify_Key\n");
    printf("Input Key Size Error\n");
    return FALSE;
  }
  else if(Key1 == NULL || Key2 == NULL){
    printf("In Verify_Key\n");
    printf("Input Key is NULL\n");
    return FALSE;
  }

  BOOLEAN Result = TRUE;

  UINT8* Key_for_hash = (Size_of_Key1 == KEY_SIZE_IN_KEY_STORE) ? Key2 : Key1;
  UINTN Size_for_hash = (Size_of_Key1 == KEY_SIZE_IN_KEY_STORE) ? Size_of_Key2 : Size_of_Key1;
  UINT8* Hash_result = NULL;
  UINT8* Another_Key = (Size_of_Key1 == KEY_SIZE_IN_KEY_STORE) ? Key1 : Key2;

  int index;

  Hash_result = (UINT8*)malloc(KEY_SIZE_IN_KEY_STORE * sizeof(UINT8));
  if(Hash_result == NULL){
    printf("In Verify_Key\n");
    printf("Hash_result Memory Allocation Error\n");
    return FALSE;
  }

  sha256_vector(1 , &Key_for_hash , &Size_for_hash , Hash_result);

  for(index = 0 ; index < KEY_SIZE_IN_KEY_STORE ; index++){
    if(Hash_result[index] != Another_Key[index]){
      Result = FALSE;
      break;
    }
  }

  return Result;
}

// My Tools
UINTN Find_Aptio_FW_Capsule_Header_Offset(char* ROM_Image_Name){
  if(ROM_Image_Name == NULL){
    printf("In Find_Aptio_FW_Capsule_Header_Offset\n");
    printf("Input Error : ROM_Image_Name\n");
    return;
  }

  // ---- Variables Declaration ----
  EFI_GUID GUID_FV_MAIN = FV_MAIN_GUID;
  EFI_GUID GUID_CAPSULE_HEADER = CAPSULE_HEADER_GUID;
  EFI_GUID GUID_SECURE = SECURE_GUID;

  UINTN FV_Offset = 0;
  UINTN FFS_Offset = 0;
  UINTN Section_Offset = 0;
  UINTN Capsule_Header_Offset = 0;

  // ---- Implementation ----
  FV_Offset = Find_Firmware_Volume_Offset(ROM_Image_Name , &GUID_FV_MAIN);
  FFS_Offset = Find_FFS_Offset(ROM_Image_Name , &FV_Offset , &GUID_CAPSULE_HEADER);
  Section_Offset = Find_Section_Offset_by_SubTypeGUID(ROM_Image_Name , &FFS_Offset , &GUID_SECURE);
  Capsule_Header_Offset = Section_Offset + sizeof(EFI_FREEFORM_SUBTYPE_GUID_SECTION);

  return Capsule_Header_Offset;
}

UINTN Find_Firmware_Volume_Offset(char* ROM_Image_Name , EFI_GUID* Taraget_FV_GUID){
  if(ROM_Image_Name == NULL){
        printf("Error : Null Rom Image Name (In GetKey.c)\n");
        return NULL;
    }

  // Declear Variables
  // ROM Image
  FILE* ROM_Image = NULL;
  UINTN ROM_Image_Size = 0;

  // Offset
  UINTN FV_Offset = -1; // Result
  UINTN Current_Offset = 0;

  // Flags
  BOOLEAN isFV;
  BOOLEAN isFound = False;

  // Header
  EFI_FIRMWARE_VOLUME_HEADER Current_FV_Header;
  EFI_FIRMWARE_VOLUME_EXT_HEADER Current_FV_Extend_Header;

  // GUID
  //EFI_GUID GUID_FV_MAIN = FV_MAIN_GUID;

  // ---Read Rom Image---
  ROM_Image = fopen(LongFilePath(ROM_Image_Name) , "rb");

  // Check if read rom image successfully
  if(ROM_Image == NULL){
    printf("Read File Fail\n");
    return NULL;
  }

  // Get Rom Image Size
  fseek(ROM_Image , 0 , SEEK_END);
  ROM_Image_Size = ftell(ROM_Image);
  fseek(ROM_Image , 0 , SEEK_SET);

  // ---Find Target Firmware Volume---
  while(Current_Offset < ROM_Image_Size){
    // Step 1. Shift to Firmware Header by "Current_Offset"
    fseek(ROM_Image , Current_Offset , SEEK_SET);

    // Step 2. Get Firmware Volume Header
    fread(&Current_FV_Header , sizeof (EFI_FIRMWARE_VOLUME_HEADER) - sizeof (EFI_FV_BLOCK_MAP_ENTRY), 1, ROM_Image);

    // Step 3. Check Signature
    isFV = Check_FV_Signature(&Current_FV_Header.Signature);

    if(isFV){
      fseek(ROM_Image , Current_Offset + Current_FV_Header.ExtHeaderOffset , SEEK_SET);
      fread(&Current_FV_Extend_Header , sizeof(EFI_FIRMWARE_VOLUME_EXT_HEADER), 1, ROM_Image);

      if(!CompareGuid(&Current_FV_Extend_Header.FvName , Taraget_FV_GUID)){
        isFound = TRUE;
        break;
      }

      Current_Offset += Current_FV_Header.FvLength;
      Current_Offset = ALIGN_POINTER(Current_Offset , 8);
    }
    else{
      Current_Offset += 0x20;
    }
  }

  fclose(ROM_Image);

  FV_Offset = (isFound) ? Current_Offset : -1;

  return (isFound) ? Current_Offset : -1;
}

UINTN Find_FFS_Offset(char* ROM_Image_Name , UINTN* FV_Offset , EFI_GUID* Target_FFS_GUID){
  if(ROM_Image_Name == NULL){
        printf("Error : Null Rom Image Name (In GetKey.c)\n");
        return NULL;
  }

  // Declear Variables
  // ROM Image
  FILE* ROM_Image = NULL;
  UINTN ROM_Image_Size = 0;

  // Offset
  UINTN FFS_Offset = -1; // Result
  UINTN Current_Offset = 0;
  UINTN FFS_Offset_Boundary = 0;

  // Flags
  BOOLEAN isFV;
  BOOLEAN isFound = FALSE;

  // Header
  EFI_FIRMWARE_VOLUME_HEADER FV_Header;
  EFI_FIRMWARE_VOLUME_EXT_HEADER FV_Extend_Header;
  EFI_FFS_FILE_HEADER Current_FFS_Header;

  // ---Read Rom Image---
  ROM_Image = fopen(LongFilePath(ROM_Image_Name) , "rb");

  // Get Rom Image Size
  fseek(ROM_Image , 0 , SEEK_END);
  ROM_Image_Size = ftell(ROM_Image);
  fseek(ROM_Image , 0 , SEEK_SET);

  // Verify Input Offset
  if(*FV_Offset >= ROM_Image_Size){
    printf("Input Firmware Volume Offset Error\n");
    return -1;
  }

  // Verfiy FV Header
  fseek(ROM_Image , *FV_Offset , SEEK_SET);
  fread(&FV_Header , sizeof (EFI_FIRMWARE_VOLUME_HEADER) - sizeof (EFI_FV_BLOCK_MAP_ENTRY) , 1, ROM_Image);
  isFV = Check_FV_Signature(&FV_Header.Signature);
  if(!isFV){
    printf("Input Firmware Volume Offset Error\n");
    return -1;
  }

  // Initialization
  fseek(ROM_Image , *FV_Offset + FV_Header.ExtHeaderOffset , SEEK_SET);
  fread(&FV_Extend_Header , sizeof(EFI_FIRMWARE_VOLUME_EXT_HEADER) , 1, ROM_Image);

  Current_Offset = *FV_Offset + FV_Header.ExtHeaderOffset + FV_Extend_Header.ExtHeaderSize;
  Current_Offset = ALIGN_POINTER(Current_Offset , 8);
  FFS_Offset_Boundary = *FV_Offset + FV_Header.FvLength;

  // Find Target FFS
  while(Current_Offset < FFS_Offset_Boundary){
    fseek(ROM_Image , Current_Offset , SEEK_SET);
    fread(&Current_FFS_Header , sizeof(EFI_FFS_FILE_HEADER), 1, ROM_Image);

    if(!CompareGuid(&Current_FFS_Header.Name , Target_FFS_GUID)){
      isFound = TRUE;
      break;
    }
    else{
      Current_Offset += Get_Length(Current_FFS_Header.Size);
      Current_Offset = ALIGN_POINTER(Current_Offset , 8);
    }
  }

  fclose(ROM_Image);

  FFS_Offset = (isFound) ? Current_Offset : -1;

  return FFS_Offset;
}

UINTN Find_Section_Offset_by_SubTypeGUID(char* ROM_Image_Name , UINTN* FFS_Offset , EFI_GUID* Target_Section_GUID){
  if(ROM_Image_Name == NULL){
        printf("Error : Null Rom Image Name (In GetKey.c)\n");
        return NULL;
  }

  // Declear Variables
  // ROM Image
  FILE* ROM_Image = NULL;
  UINTN ROM_Image_Size = 0;

  // Offset
  UINTN Section_Offset = -1; // Result
  UINTN Current_Offset = 0;
  UINTN Section_Offset_Boundary = 0;

  // Flags
  BOOLEAN isFound = FALSE;

  // Header
  EFI_FFS_FILE_HEADER FFS_Header;
  EFI_FREEFORM_SUBTYPE_GUID_SECTION Current_Section_Header;

  // ---Read Rom Image---
  ROM_Image = fopen(LongFilePath(ROM_Image_Name) , "rb");

  // Get Rom Image Size
  fseek(ROM_Image , 0 , SEEK_END);
  ROM_Image_Size = ftell(ROM_Image);
  fseek(ROM_Image , 0 , SEEK_SET);

  // Verify Input Offset
  if(*FFS_Offset >= ROM_Image_Size){
    printf("Input FFS Offset Error\n");
    return -1;
  }

  // Initialization
  fseek(ROM_Image , *FFS_Offset , SEEK_SET);
  fread(&FFS_Header , sizeof(EFI_FFS_FILE_HEADER) , 1, ROM_Image);

  Current_Offset = *FFS_Offset + sizeof(EFI_FFS_FILE_HEADER);
  Current_Offset = ALIGN_POINTER(Current_Offset , 8);
  Section_Offset_Boundary = *FFS_Offset + Get_Length(FFS_Header.Size);

  // Find Target FFS
  while(Current_Offset < Section_Offset_Boundary){
    fseek(ROM_Image , Current_Offset , SEEK_SET);
    fread(&Current_Section_Header , sizeof(EFI_FREEFORM_SUBTYPE_GUID_SECTION), 1, ROM_Image);

    if(!CompareGuid(&Current_Section_Header.SubTypeGuid , Target_Section_GUID)){
      isFound = TRUE;
      break;
    }
    else{
      Current_Offset += Get_Length(Current_Section_Header.CommonHeader.Size);
      Current_Offset = ALIGN_POINTER(Current_Offset , 8);
    }
  }

  fclose(ROM_Image);

  Section_Offset = (isFound) ? Current_Offset : -1;

  return Section_Offset;
}

void Save_Key(UINT8*** Keys , UINTN index_of_keys , char* path , UINTN Size){
  FILE *output_file;

  output_file = fopen(path , "wb");

  if(output_file == NULL){
    printf("Error : Fail to open file\n");
    return;
  }

  fwrite(Public_Key_Header , sizeof(UINT8) , sizeof(Public_Key_Header) , output_file);
  fwrite((*Keys)[index_of_keys] , sizeof(UINT8) , Size , output_file);
  fwrite(Public_Key_Tail , sizeof(UINT8) , sizeof(Public_Key_Tail) , output_file);

  fclose(output_file);
}

void Save_Signature(UINT8*** Signatures , UINTN index_of_keys , char* path , UINTN Size){
  FILE *output_file;

  output_file = fopen(path , "wb");

  if(output_file == NULL){
    printf("Error : Fail to open file\n");
    return;
  }

  fwrite((*Signatures)[index_of_keys] , sizeof(UINT8) , Size , output_file);

  fclose(output_file);
}


BOOLEAN Check_FV_Signature(UINT32* Input){
  BOOLEAN result = TRUE;

  if(*Input != EFI_FVH_SIGNATURE){
    result = FALSE;
  }

  return result;
}

int Get_Number_of_Key(EFI_FFS_FILE_HEADER* input){
  int number_of_keys = 0;
  int file_size = 0;
  int data_size = 0;

  int index;

  
  for(index = 2 ; index >= 0 ; index--){
    file_size <<= 8;
    file_size += input->Size[index];
  }

  data_size = file_size - sizeof(EFI_FFS_FILE_HEADER) - 4;
  number_of_keys = data_size / 32;
  
  return number_of_keys;
}

UINTN Get_Length(UINT8* array){
  UINTN length = 0;

  for(int index = 2 ; index >= 0 ; index--){
    length <<= 8;
    length += array[index];
  }

  return length;
}

// For checking
void Print_FV_Header(EFI_FIRMWARE_VOLUME_HEADER* FV_Header){
  if(FV_Header == NULL){
    return;
  }

  // Print GUID
  printf("Guid : ");
  Print_GUID(&(FV_Header->FileSystemGuid));
  // Print Firmware Volume Size
  printf("Firmware Volume Size : %lX \n" , FV_Header->FvLength);

  // Print Signature
  printf("Signature : ");
  Print_Signature(&FV_Header->Signature);

  // Print Attribute
  printf("Attribute : ");
  Print_Attribute(&FV_Header->Attributes);

  // Print Header Size
  printf("Header Size : %X\n" , FV_Header->HeaderLength);

  // Print Checksum
  printf("Check Sum : %X\n" , FV_Header->Checksum);

  // Print Extend Header Offset
  printf("Extend Header Offset : %X\n" , FV_Header->ExtHeaderOffset);

  // Print Revision
  printf("Revision : %X\n" , FV_Header->Revision);

}

void Print_GUID(EFI_GUID* guid){
  printf("%X - " , guid->Data1);
  printf("%X - " , guid->Data2);
  printf("%X - " , guid->Data3);
  
  for(int index = 0 ; index < 2 ; index++){
    printf("%X" , guid->Data4[index]);
  }
  printf(" - ");
  for(int index = 2 ; index < 8 ; index++){
    printf("%X" , guid->Data4[index]);
  }
  printf("\n");
}

void Print_Signature(UINT32* input){
  UINT32 dummy = *input;
  char current_character;
  int index;

  for(index = 0 ; index < 4 ; index++){
    current_character = dummy & 0xff;
    dummy >>= 8;
    
    printf("%c" , current_character);
  }
  printf("\n");
}

void Print_Attribute(EFI_FVB_ATTRIBUTES_2* input){
  UINT8 attribute[4] = {0};

  UINT32 dummy = *input;

  int index;

  for(index = 0 ; index < 4 ; index++){
    attribute[index] = dummy & 0xff;
    dummy >>= 8;
  }
  for(index = 3 ; index >= 0 ; index--){
    printf("%X" , attribute[index]);
  }
  printf("\n");
}

void Print(UINT8* array , UINTN size){
  int current_index;
  
  for(current_index = 0 ; current_index < size ; current_index++){
    if(0 != current_index && 0 == current_index % 16){
      printf("\n");
    }

    printf("%02X " , array[current_index]);
  }

  printf("\n");
}

void Print_Rom_Layout(char* ROM_Image_Name , UINTN Capsule_Header_Offset , BOOLEAN Show_Material_for_Verify){
  // ---- Variables Declaration ----
  FILE* ROM_Image = NULL;
  
  APTIO_FW_CAPSULE_HEADER Capsule_Header;

  ROM_AREA* RomLayout_Table;
  UINTN Rom_Layout_Offset = 0;
  UINTN Rom_Layout_Size = 0;
  UINTN Rom_Layout_Max_Number = 0;
  UINTN Number_of_Material_from_Rom_Layout = 0;

  int current_index;

  // ---- Implementation ----
  ROM_Image = fopen(LongFilePath(ROM_Image_Name) , "rb");
  if(ROM_Image == NULL){
    printf("In Get_Hash_Material_According_to_Rom_Map\n");
    printf("Open File Error\n");
    return;
  }

  fseek(ROM_Image , Capsule_Header_Offset , SEEK_SET);
  fread(&Capsule_Header , sizeof (APTIO_FW_CAPSULE_HEADER), 1, ROM_Image);

  Rom_Layout_Offset = Capsule_Header_Offset + Capsule_Header.RomLayoutOffset;
  Rom_Layout_Size = Capsule_Header.RomImageOffset - Capsule_Header.RomLayoutOffset;
  Rom_Layout_Max_Number = Rom_Layout_Size / sizeof(ROM_AREA);
  RomLayout_Table = (ROM_AREA*)malloc(Rom_Layout_Max_Number * sizeof(ROM_AREA));
  fseek(ROM_Image , Rom_Layout_Offset , SEEK_SET);
  fread(RomLayout_Table , sizeof (ROM_AREA), Rom_Layout_Max_Number, ROM_Image);

  for(current_index = 0 ; current_index < Rom_Layout_Max_Number ; current_index++){
    if(RomLayout_Table[current_index].Attributes & AMI_ROM_AREA_SIGNED){
      Number_of_Material_from_Rom_Layout++;
    }
  }

  printf("Number of Rom for Verify : %d\n" , Number_of_Material_from_Rom_Layout);


  UINTN test = 0;
  if(Show_Material_for_Verify){
    for(current_index = 0 ; current_index < Rom_Layout_Max_Number ; current_index++){
      if(RomLayout_Table[current_index].Attributes & AMI_ROM_AREA_SIGNED && Show_Material_for_Verify){
        printf("Rom Index : %d\n" , current_index);
        printf("Physical Address : %X\n" , RomLayout_Table[current_index].Address);
        printf("Offset %X\n" , RomLayout_Table[current_index].Offset);
        printf("Size : %X\n" , RomLayout_Table[current_index].Size);
        printf("Attributes : %X\n" , RomLayout_Table[current_index].Attributes);
        printf("---\n");
        test += (RomLayout_Table[current_index].Size);
      }
    }
    printf("Total Size : %X\n",test);
    printf("Total Size : %dss\n",test);
  }
  else{
    for(current_index = 0 ; current_index < Rom_Layout_Max_Number ; current_index++){
      if(TRUE){
        printf("Rom Index : %d\n" , current_index);
        printf("Physical Address : %X\n" , RomLayout_Table[current_index].Address);
        printf("Offset %X\n" , RomLayout_Table[current_index].Offset);
        printf("Size : %X\n" , RomLayout_Table[current_index].Size);
        printf("Attributes : %X\n" , RomLayout_Table[current_index].Attributes);
        printf("---\n");
      }
    }
  }
}

// UI
void My_UI(int argc , char* argv[]){
  // Check instructions
  if(argc < 3){
    UI_Help(NULL);
    return 0;
  }

  for(int current_index = 0 ; current_index < sizeof(Instructions) / sizeof(char*) ;){
    if(!strcmp(argv[1] , Instructions[current_index])){
      break;
    }

    current_index++;
    if(current_index >= sizeof(Instructions) / sizeof(char*)){
      UI_Help(NULL);
      return 0;
    }
  }

  // Functions
  if(!strcmp(argv[1] , Instruction_Help)){
    UI_Help(NULL);
    /*
    if(argc == 3){
      UI_Help(argv[2]);
    }
    else{
      UI_Help(NULL);
    }
    */
  }
  else if(!strcmp(argv[1] , Instruction_Get_Key_and_Signature_from_Capsule_Header)){
    UI_Get_Key_and_Signature_from_Capsule_Header(argc , argv);
  }
  else if(!strcmp(argv[1] , Instruction_Get_Key_from_Key_Store)){
    UI_Get_Key_from_Key_Store(argc , argv);
  }
  else if(!strcmp(argv[1] , Instruction_Get_Rom_Layout)){
    UI_Get_Rom_Layout(argc , argv);
  }
  else if(!strcmp(argv[1] , Instruction_Verify_Key)){
    UI_Verify_Key(argc , argv);
  }
}

void UI_Help(char* instruction){
  int number_of_instructions = sizeof(Instructions) / sizeof(char*);
  int current_instruction_index;

  if(instruction == NULL){
    printf("---Instructions List---\n");

    for(current_instruction_index = 0 ; current_instruction_index < number_of_instructions ; current_instruction_index++){
      printf("%d. %s\n" , current_instruction_index + 1 , Instruction_Name[current_instruction_index]);
      printf("%s " , Instructions[current_instruction_index]);
      printf("%s\n\n" , Instruction_Usage[current_instruction_index]);
    }
  }

  // 仿 system("Pause")
    printf("Press Enter key to continue...\n");  
    fgetc(stdin);
}

void UI_Get_Key_and_Signature_from_Capsule_Header(int argc , char* argv[]){
  if(argc != 3 && argc != 4){
    printf("Input Error\n");
    return;
  }

  BOOLEAN save = FALSE;
  if(argc == 4){
    save = (!strcmp(argv[3] , "-save")) ? TRUE : FALSE;
  }

  UINT8** Keys = NULL;
  UINT8** Signatures = NULL;

  int current_key_index;

  Get_Keys_and_Signatures_from_Capsule_Header(argv[2] , &Keys , &Signatures);

  // Print Keys
  for(current_key_index = 0 ; current_key_index < 2 ; current_key_index++){
    printf("Key %d : \n" , current_key_index);
    Print(Keys[current_key_index] , KEY_SIZE_IN_CAPSULE);
    printf("\n");
  }

  // Print Signatures
  for(current_key_index = 0 ; current_key_index < 2 ; current_key_index++){
    printf("Signature %d : \n" , current_key_index);
    Print(Signatures[current_key_index] , SIGNATURE_SIZE_IN_CAPSULE);
    printf("\n");
  }

  if(save){
    Save_Key(&Keys , 0 , "PublicKey_Header_Sign.bin" , KEY_SIZE_IN_CAPSULE);
    Save_Key(&Keys , 1 , "PublicKey_Header_Root.bin" , KEY_SIZE_IN_CAPSULE);
    Save_Signature(&Signatures , 0 , "Cert_Header_Sign.bin" , SIGNATURE_SIZE_IN_CAPSULE);
    Save_Signature(&Signatures , 1 , "Cert_Header_Root.bin" , SIGNATURE_SIZE_IN_CAPSULE);
  }

  free(Keys);
  free(Signatures);
}

void UI_Get_Key_from_Key_Store(int argc , char* argv[]){
  if(argc != 3 && argc != 4){
    printf("Input Error\n");
    return;
  }

  BOOLEAN save = FALSE;
  if(argc == 4){
    save = (!strcmp(argv[3] , "-save")) ? TRUE : FALSE;
  }

  UINT8** Keys = NULL;
  UINTN number_of_keys;

  int current_key_index;
  int current_value;

  Get_Keys_from_Key_Store(argv[2] , &Keys, &number_of_keys);

  if(Keys == NULL){
    printf("Key Store is empty\n");
    return;
  }
  else{
    for(current_key_index = 0 ; current_key_index < number_of_keys ; current_key_index++){
      printf("Key %d : \n" , current_key_index);
      Print(Keys[current_key_index] , KEY_SIZE_IN_KEY_STORE);
      printf("\n");
    }
  }

  if(save){
    Save_Key(&Keys , 0 , "PublicKey_KeyStore.bin"  , KEY_SIZE_IN_KEY_STORE);
  }

  free(Keys);
}

void UI_Get_Rom_Layout(int argc , char* argv[]){
  if(argc != 3 && argc != 4){
    printf("Input Error\n");
    return;
  }

  UINT8** Materials = NULL;
  UINTN* Materials_Size = NULL;
  UINT8 Number_of_Materials = 0;

  UINTN Capsule_Offset = Find_Aptio_FW_Capsule_Header_Offset(argv[2]);

  FILE *output_file;

  BOOLEAN save = (argc == 4 && !strcmp(argv[3] , "-save")) ? TRUE : FALSE;
  int index;

  Get_Rom_for_Verify(argv[2] , Capsule_Offset , &Materials , &Materials_Size , &Number_of_Materials);

  if(Materials != NULL && Materials_Size != NULL){
    printf("Numbe of Materials : %d\n\n" , Number_of_Materials);
    for(index = 0 ; index < Number_of_Materials ; index++){
      printf("Material %d \n" , index);
    }
  }
  else{
    return;
  }

  if(save && Materials != NULL && Materials_Size != NULL){
    output_file = fopen("Rom_for_Verify.bin" , "wb");
    if(output_file == NULL){
      printf("Error : Fail to open file\n");
      return;
    }
    for(index = 0 ; index < Number_of_Materials ; index++){
      fwrite(Materials[index], sizeof(UINT8), Materials_Size[index] , output_file);
    }
    fclose(output_file);
  }

  return;
}

void UI_Verify_Key(int argc , char* argv[]){
  if(argc != 4){
    printf("Input Error\n");
    return;
  }

  BOOLEAN Result = FALSE;

  FILE* Current_file;

  UINT8* Key1 = NULL;
  UINT8* Key2 = NULL;
  UINTN Size_of_Key1 = 0;
  UINTN Size_of_Key2 = 0;

  Current_file = NULL;
  Current_file = fopen(argv[2] , "rb");
  if(Current_file == NULL){
    printf("In UI_Verify_Key\n");
    printf("Open File Error\n");
    return;
  }

  fseek(Current_file , 0 , SEEK_END);
  Size_of_Key1 = ftell(Current_file);
  fseek(Current_file , 0 , SEEK_SET);

  Key1 = (UINT8*)malloc(Size_of_Key1 * sizeof(UINT8));
  if(Key1 == NULL || Size_of_Key1 == 0){
    printf("In UI_Verify_Key\n");
    printf("Read Key Error\n");
  }
  fread(Key1 , sizeof(UINT8) , Size_of_Key1 , Current_file);

  fclose(Current_file);

  Current_file = NULL;
  Current_file = fopen(argv[3] , "rb");
  if(Current_file == NULL){
    printf("In UI_Verify_Key\n");
    printf("Open File Error\n");
    return;
  }

  fseek(Current_file , 0 , SEEK_END);
  Size_of_Key2 = ftell(Current_file);
  fseek(Current_file , 0 , SEEK_SET);

  Key2 = (UINT8*)malloc(Size_of_Key2 * sizeof(UINT8));
  if(Key2 == NULL || Size_of_Key2 == 0){
    printf("In UI_Verify_Key\n");
    printf("Read Key Error\n");
  }
  fread(Key2 , sizeof(UINT8) , Size_of_Key2 , Current_file);

  fclose(Current_file);

  Result = Verify_Key(Key1 , Size_of_Key1 , Key2 , Size_of_Key2);

  if(Result){
    printf("Verified Success\n");
  }
  else{
    printf("Verified Failure\n");
  }
}