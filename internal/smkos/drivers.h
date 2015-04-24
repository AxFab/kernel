#pragma once

/* Those are major driver number - they're used only inside a driver but we 
 * keep them here for listing */
#define ISO_No 12
#define GPT_No 35
#define ATA_No 41
#define HDD_No 42
#define VGA_No 23
#define KDB_No 38


/* --- File Systems ------------------------------------------------------ */
void TMPFS(kDriver_t *driver);
void GPT(kDriver_t *driver);
void ISO9660(kDriver_t *driver);
void FATFS(kDriver_t *driver);
/* --- Block drivers ----------------------------------------------------- */
void ATA(kDriver_t *driver);
void VGA(kDriver_t *driver);
void HDD(kDriver_t *driver);
/* --- Char drivers ------------------------------------------------------ */
void KDB(kDriver_t *driver);


/* --- Register all drivers ---------------------------------------------- */
static inline void init_driver() 
{
  register_driver(GPT);
  register_driver(ISO9660);
  /* register_driver(FATFS); */

#if 1 /* _x86 */
  register_driver(ATA);
  register_driver(VGA);
  register_driver(KDB);
#else /* _um */
  register_driver(HDD);
  register_driver(BMP);
#endif
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
