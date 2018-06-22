#ifdef _WIN32
#include <stdlib.h>
#include <windows.h>
#endif

#if __linux__ || __APPLE__
#include <cstdlib>
#include <gtk/gtk.h>
#endif

#include "vars.h"
#include <SDL.h>
#include <cstdio>

#ifdef _WIN32
char filename[100]; // The filename

BOOL WINAPI GetOpenFileName(_Inout_ LPOPENFILENAME lpofn);

// Opens a Win32 window to pick a file
INT chooseProgram(PCHAR FileName) {
    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = "Chip8 ROMs\0*.ch8\0All Files\0*.*\0\0";
    ofn.lpstrFile = FileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Please Select A CHIP8 ROM";
    ofn.Flags = OFN_NONETWORKBUTTON | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    if (!GetOpenFileName(&ofn)) {
        return 0;
    }
    return 1;
}

// Loads game to memory
void loadgame() {
    FILE *space;
    chooseProgram(filename);  // Opens a window to choose ROM

    if (fopen_s(&space, filename, "rb") != 0) {
        printf("failed");
        exit(0);
    }

    fseek(space, 0, SEEK_END);
    long bufferSize = ftell(space);
    rewind(space);
    unsigned char *buffer = (unsigned char *)malloc(sizeof(char) * bufferSize);
    size_t result = fread(buffer, 1, bufferSize, space);

    for (uint32_t i = 0; i < bufferSize; ++i) {  // Load ROM at 0x200
        memory.at(i + 512) = buffer[i];
    }

    fclose(space);
    free(buffer);
}
#endif

#if __linux__ || __APPLE__

char* chooseProgram() {
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Open File", NULL, action, ("_Cancel"),
            GTK_RESPONSE_CANCEL, ("_Open"),
            GTK_RESPONSE_ACCEPT, NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        char* filename = gtk_file_chooser_get_filename(chooser);
    }
    gtk_widget_destroy(dialog);

    return filename;
}

// Loads game to memory
void loadgame() {
    FILE *space;
    // Opens a window to choose ROM
    space = fopen(chooseProgram(), "rb");

    fseek(space, 0, SEEK_END);
    int64_t bufferSize = ftell(space);
    rewind(space);

    auto *buffer = static_cast<uint8_t *>(malloc(sizeof(char) * bufferSize));
    uint32_t result = fread(buffer, 1, bufferSize, space);

    for (int i = 0; i < bufferSize; ++i) { // Load ROM at 0x200
        memory.at(i + 512) = buffer[i];
    }

    fclose(space);
    free(buffer);
}
#endif
