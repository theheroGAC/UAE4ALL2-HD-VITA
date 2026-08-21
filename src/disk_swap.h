#ifndef DISK_SWAP_H
#define DISK_SWAP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Restituisce il numero del disco attualmente inserito in DF0 (1 o 2).
 */
int GetCurrentDisk(void);

/**
 * Esegue lo swap del dischetto in DF0 (Disk 1 <-> Disk 2).
 * Controlla l'esistenza del file e attiva l'indicatore OSD.
 */
void SwapDisk(void);

#ifdef __cplusplus
}
#endif

#endif // DISK_SWAP_H
