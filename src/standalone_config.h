#ifndef STANDALONE_CONFIG_H
#define STANDALONE_CONFIG_H

#include <SDL.h>
#include <stdbool.h>

#define GAME_PATH "ux0:/data/MioGiocoAmiga/"
#define ROM_NAME "kick3.rom"
#define DISK1_NAME "disk1.adf"
#define DISK2_NAME "disk2.adf"

#define PATH_KICK3 GAME_PATH ROM_NAME
#define PATH_DISK1 GAME_PATH DISK1_NAME
#define PATH_DISK2 GAME_PATH DISK2_NAME

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Verifica se un file esiste sul filesystem.
 */
bool FileExists(const char *path);

/**
 * Mostra a schermo una finestra di errore formattata con SDL, attende 5 secondi e chiude in modo sicuro.
 */
void ShowErrorAndExit(const char *line1, const char *line2, const char *line3, const char *line4);

/**
 * Esegue il controllo al boot dei file richiesti (kick3.rom, disk1.adf, disk2.adf).
 */
bool Standalone_CheckBootFiles(void);

/**
 * Configura i parametri UAE4All2 per l'avvio immediato con Kickstart 3.x e DF0 caricato.
 */
void Standalone_ConfigureEmulator(void);

#ifdef __cplusplus
}
#endif

#endif // STANDALONE_CONFIG_H
