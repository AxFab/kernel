#pragma once
#include <smkos/kernel.h>

void TMPFS(kDriver_t *);
void ISO9660(kDriver_t *);
void FATFS(kDriver_t *);
void ATA(kDriver_t *);
void HDD(kDriver_t *);
void GPT(kDriver_t *);

#define HDD NULL

