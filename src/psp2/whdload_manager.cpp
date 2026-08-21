#ifdef __PSP2__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/stat.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#include <archive.h>
#include <archive_entry.h>

#include "whdload_manager.h"

static int path_exists(const char *path, int *is_dir)
{
    SceIoStat st;
    if (sceIoGetstat(path, &st) < 0)
        return 0;
    if (is_dir)
        *is_dir = SCE_S_ISDIR(st.st_mode) ? 1 : 0;
    return 1;
}

static void ensure_directory(const char *path)
{
    char buffer[512];
    size_t len;
    strncpy(buffer, path, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    len = strlen(buffer);
    while (len > 0 && buffer[len - 1] == '/') {
        buffer[len - 1] = '\0';
        len--;
    }
    for (char *p = buffer; *p; p++) {
        if (*p == '/' && p != buffer + 4) {
            *p = '\0';
            sceIoMkdir(buffer, 0777);
            *p = '/';
        }
    }
    sceIoMkdir(buffer, 0777);
}

static int make_safe_relative(const char *source, char *destination, size_t destination_size)
{
    if (!source || !destination || destination_size == 0 || source[0] == '/' || source[0] == '\\' || strchr(source, ':'))
        return 0;

    size_t out = 0;
    const char *part = source;
    while (*part) {
        while (*part == '/') part++;
        if (!*part) break;
        const char *end = strchr(part, '/');
        size_t length = end ? (size_t)(end - part) : strlen(part);
        if (length == 0 || length == 1 && part[0] == '.' || length == 2 && part[0] == '.' && part[1] == '.')
            return 0;
        if (out != 0) {
            if (out + 1 >= destination_size) return 0;
            destination[out++] = '/';
        }
        if (out + length >= destination_size) return 0;
        memcpy(destination + out, part, length);
        out += length;
        part = end ? end + 1 : part + length;
    }
    if (out == 0) return 0;
    destination[out] = '\0';
    return 1;
}

static void make_folder_name(const char *archive_path, char *folder, size_t folder_size)
{
    const char *name = strrchr(archive_path, '/');
    name = name ? name + 1 : archive_path;
    strncpy(folder, name, folder_size - 1);
    folder[folder_size - 1] = '\0';
    char *dot = strrchr(folder, '.');
    if (dot) *dot = '\0';
    for (size_t i = 0; folder[i]; i++) {
        unsigned char c = (unsigned char)folder[i];
        if (!isalnum(c) && c != '_' && c != '-' && c != ' ' && c != '.')
            folder[i] = '_';
    }
    if (folder[0] == '\0')
        strncpy(folder, "game", folder_size - 1);
    folder[folder_size - 1] = '\0';
}

static void make_unique_folder(const char *base, char *folder, size_t folder_size)
{
    char candidate[512];
    int number = 1;
    strncpy(folder, base, folder_size - 1);
    folder[folder_size - 1] = '\0';
    snprintf(candidate, sizeof(candidate), "%s/%s", VITA_WHDLOAD_ROOT, folder);
    int is_dir = 0;
    while (path_exists(candidate, &is_dir)) {
        number++;
        snprintf(folder, folder_size, "%s_%d", base, number);
        snprintf(candidate, sizeof(candidate), "%s/%s", VITA_WHDLOAD_ROOT, folder);
    }
}

static int write_entry(struct archive *input, const char *path)
{
    FILE *output = fopen(path, "wb");
    if (!output)
        return 0;
    char buffer[65536];
    la_ssize_t bytes;
    int success = 1;
    while ((bytes = archive_read_data(input, buffer, sizeof(buffer))) > 0) {
        if (fwrite(buffer, 1, (size_t)bytes, output) != (size_t)bytes) {
            success = 0;
            break;
        }
    }
    if (bytes < 0)
        success = 0;
    fclose(output);
    return success;
}

static int find_slave_recursive(const char *root, const char *relative, char *result, size_t result_size, int depth)
{
    if (depth > 8)
        return 0;

    char directory_path[512];
    if (relative && relative[0])
        snprintf(directory_path, sizeof(directory_path), "%s/%s", root, relative);
    else
        snprintf(directory_path, sizeof(directory_path), "%s", root);

    SceUID directory = sceIoDopen(directory_path);
    if (directory < 0)
        return 0;

    SceIoDirent entry;
    int found = 0;
    while (!found && sceIoDread(directory, &entry) > 0) {
        if (entry.d_name[0] == '.' || strcmp(entry.d_name, "S") == 0)
            continue;

        char child_relative[384];
        if (relative && relative[0])
            snprintf(child_relative, sizeof(child_relative), "%s/%s", relative, entry.d_name);
        else
            snprintf(child_relative, sizeof(child_relative), "%s", entry.d_name);

        if (SCE_S_ISDIR(entry.d_stat.st_mode)) {
            found = find_slave_recursive(root, child_relative, result, result_size, depth + 1);
        } else {
            const char *dot = strrchr(entry.d_name, '.');
            if (dot && strcasecmp(dot, ".slave") == 0) {
                strncpy(result, child_relative, result_size - 1);
                result[result_size - 1] = '\0';
                found = 1;
            }
        }
    }
    sceIoDclose(directory);
    return found;
}

static int copy_text_file(const char *source, const char *destination)
{
    FILE *input = fopen(source, "rb");
    if (!input)
        return 0;
    FILE *output = fopen(destination, "wb");
    if (!output) {
        fclose(input);
        return 0;
    }
    char buffer[4096];
    size_t bytes;
    int success = 1;
    while ((bytes = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        if (fwrite(buffer, 1, bytes, output) != bytes) {
            success = 0;
            break;
        }
    }
    fclose(input);
    fclose(output);
    return success;
}

static int read_text_file(const char *path, char *buffer, size_t buffer_size)
{
    FILE *input = fopen(path, "rb");
    if (!input)
        return 0;
    size_t length = fread(buffer, 1, buffer_size - 1, input);
    fclose(input);
    buffer[length] = '\0';
    return 1;
}

static int write_text_file(const char *path, const char *text)
{
    FILE *output = fopen(path, "wb");
    if (!output)
        return 0;
    size_t length = strlen(text);
    int success = fwrite(text, 1, length, output) == length;
    fclose(output);
    return success;
}

static char *find_endcli(char *text)
{
    for (char *p = text; *p; p++) {
        if ((p == text || p[-1] == '\n') &&
            (p[0] == 'E' || p[0] == 'e') &&
            (p[1] == 'N' || p[1] == 'n') &&
            (p[2] == 'D' || p[2] == 'd') &&
            (p[3] == 'C' || p[3] == 'c') &&
            (p[4] == 'L' || p[4] == 'l') &&
            (p[5] == 'I' || p[5] == 'i'))
            return p;
    }
    return NULL;
}

const char *vita_whdload_root(void)
{
    return VITA_WHDLOAD_ROOT;
}

int vita_whdload_install_lha(const char *archive_path, char *installed_path, size_t installed_path_size)
{
    if (!archive_path || !installed_path || installed_path_size == 0)
        return 0;

    installed_path[0] = '\0';
    ensure_directory(VITA_WHDLOAD_ROOT);

    char base[128];
    char folder[128];
    make_folder_name(archive_path, base, sizeof(base));
    make_unique_folder(base, folder, sizeof(folder));

    char destination_root[512];
    snprintf(destination_root, sizeof(destination_root), "%s/%s", VITA_WHDLOAD_ROOT, folder);
    ensure_directory(destination_root);

    struct archive *input = archive_read_new();
    struct archive_entry *entry = NULL;
    if (!input)
        return 0;
    archive_read_support_filter_all(input);
    archive_read_support_format_lha(input);
    if (archive_read_open_filename(input, archive_path, 64 * 1024) != ARCHIVE_OK) {
        archive_read_free(input);
        return 0;
    }

    int extracted = 0;
    int success = 1;
    while (archive_read_next_header(input, &entry) == ARCHIVE_OK) {
        const char *entry_name = archive_entry_pathname(entry);
        char relative[384];
        if (!make_safe_relative(entry_name, relative, sizeof(relative))) {
            success = 0;
            break;
        }

        char output_path[512];
        snprintf(output_path, sizeof(output_path), "%s/%s", destination_root, relative);
        mode_t file_type = archive_entry_filetype(entry);
        if (file_type == AE_IFDIR) {
            ensure_directory(output_path);
            continue;
        }
        if (file_type != AE_IFREG) {
            archive_read_data_skip(input);
            continue;
        }

        char *slash = strrchr(output_path, '/');
        if (slash) {
            *slash = '\0';
            ensure_directory(output_path);
            *slash = '/';
        }
        if (!write_entry(input, output_path)) {
            success = 0;
            break;
        }
        extracted++;
    }

    archive_read_free(input);
    if (!success || extracted == 0)
        return 0;

    strncpy(installed_path, destination_root, installed_path_size - 1);
    installed_path[installed_path_size - 1] = '\0';
    return 1;
}

int vita_whdload_prepare_launch(const char *game_name)
{
    if (!game_name || game_name[0] == '\0' || strchr(game_name, '/') || strchr(game_name, '\\') || strchr(game_name, ':'))
        return 0;

    char game_root[512];
    char slave_relative[384];
    snprintf(game_root, sizeof(game_root), "%s/%s", VITA_WHDLOAD_ROOT, game_name);
    int is_dir = 0;
    if (!path_exists(game_root, &is_dir) || !is_dir)
        return 0;
    if (!find_slave_recursive(game_root, "", slave_relative, sizeof(slave_relative), 0))
        return 0;

    char startup_dir[512];
    char startup_path[512];
    char backup_path[512];
    char launch_script_path[512];
    snprintf(startup_dir, sizeof(startup_dir), "%s/S", VITA_WHDLOAD_ROOT);
    snprintf(startup_path, sizeof(startup_path), "%s/Startup-Sequence", startup_dir);
    snprintf(backup_path, sizeof(backup_path), "%s/Startup-Sequence.uae4all", startup_dir);
    snprintf(launch_script_path, sizeof(launch_script_path), "%s/UAE4ALL-WHDLoad", startup_dir);
    ensure_directory(startup_dir);

    int startup_exists = path_exists(startup_path, NULL);
    if (startup_exists && !path_exists(backup_path, NULL)) {
        if (!copy_text_file(startup_path, backup_path))
            return 0;
    }

    char amiga_slave[512];
    snprintf(amiga_slave, sizeof(amiga_slave), "DH0:%s/%s", game_name, slave_relative);
    char launch_script[640];
    snprintf(launch_script, sizeof(launch_script), "C:WHDLoad \"%s\"\n", amiga_slave);
    if (!write_text_file(launch_script_path, launch_script))
        return 0;

    char original_startup[65536];
    original_startup[0] = '\0';
    if (path_exists(backup_path, NULL)) {
        if (!read_text_file(backup_path, original_startup, sizeof(original_startup)))
            return 0;
    } else if (startup_exists) {
        if (!read_text_file(startup_path, original_startup, sizeof(original_startup)))
            return 0;
    }

    const char *execute_line = "Execute S:UAE4ALL-WHDLoad\n";
    if (strstr(original_startup, "UAE4ALL-WHDLoad") == NULL) {
        char updated_startup[66000];
        char *endcli = find_endcli(original_startup);
        if (endcli) {
            size_t prefix_length = (size_t)(endcli - original_startup);
            if (prefix_length + strlen(execute_line) + strlen(endcli) + 1 >= sizeof(updated_startup))
                return 0;
            memcpy(updated_startup, original_startup, prefix_length);
            memcpy(updated_startup + prefix_length, execute_line, strlen(execute_line));
            strcpy(updated_startup + prefix_length + strlen(execute_line), endcli);
        } else {
            if (strlen(original_startup) + strlen(execute_line) + 1 >= sizeof(updated_startup))
                return 0;
            strcpy(updated_startup, original_startup);
            strcat(updated_startup, execute_line);
        }
        if (!write_text_file(startup_path, updated_startup))
            return 0;
    }

    return 1;
}

int vita_whdload_list(char names[][128], int max_names)
{
    if (!names || max_names <= 0)
        return 0;

    ensure_directory(VITA_WHDLOAD_ROOT);
    SceUID directory = sceIoDopen(VITA_WHDLOAD_ROOT);
    if (directory < 0)
        return 0;

    int count = 0;
    SceIoDirent entry;
    while (count < max_names && sceIoDread(directory, &entry) > 0) {
        if (entry.d_name[0] == '.')
            continue;
        if (!SCE_S_ISDIR(entry.d_stat.st_mode))
            continue;
        strncpy(names[count], entry.d_name, 127);
        names[count][127] = '\0';
        count++;
    }
    sceIoDclose(directory);
    return count;
}

#endif
