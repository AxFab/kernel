#pragma once


#define ISO_No 12
#define GPT_No 35
#define ATA_No 41
#define HDD_No 42
#define VGA_No 23
#define KDB_No 38



void TMPFS(kDriver_t *driver);
void ATA(kDriver_t *driver);
void HDD(kDriver_t *driver);
void KDB(kDriver_t *driver);
void VGA(kDriver_t *driver);
void ISO9660(kDriver_t *driver);
void GPT(kDriver_t *driver);
// int fs_drivers() 
// {
#define HDD NULL


//   return 0;
// }

