#ifdef _WIN32
#include <windows.h>
#include <stdlib.h>
#include <SDL.h>
#endif
#ifdef linux
#include <cstdlib>
#include <gtk/gtk.h>
#include "SDL.h"
#endif
#include <stdio.h>
//#include <string.h>
//#include <conio.h>
#include "vars.h"

#ifdef _WIN32
char filename[100]; //The filename

BOOL WINAPI GetOpenFileName(
	_Inout_  LPOPENFILENAME lpofn
	);

//Opens a Win32 window to pick a file
INT chooseProgram(PCHAR FileName)
{
	OPENFILENAME  ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = "Chip8 ROMs\0*.ch8\0All Files\0*.*\0\0";
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = "Please Select A CHIP8 ROM";
	ofn.Flags = OFN_NONETWORKBUTTON |
		OFN_FILEMUSTEXIST |
		OFN_HIDEREADONLY;
	if (!GetOpenFileName(&ofn))
		return(0);
	return 1;
}

//Loads game to memory
void loadgame(){
	FILE *space;
	chooseProgram(filename); //Opens a window to choose ROM
	if (fopen_s(&space, filename, "rb") != 0){
		printf("failed");
		exit(0);
	}
	fseek(space, 0, SEEK_END);
	long bufferSize = ftell(space);
	rewind(space);
	unsigned char * buffer = (unsigned char *)malloc(sizeof(char) * bufferSize);
	size_t result = fread(buffer, 1, bufferSize, space);
	for (int i = 0; i < bufferSize; ++i){ //Load ROM at 0x200
		memory[i + 512] = buffer[i];
	}
	fclose(space);
	free(buffer);
}
#endif
#ifdef linux
char *filename;

void chooseProgram(){
	
	GtkWidget *dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	gint res;

	dialog = gtk_file_chooser_dialog_new("Open File",
		NULL, action, ("_Cancel"), GTK_RESPONSE_CANCEL,
		("_Open"),
		GTK_RESPONSE_ACCEPT,
		NULL);

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_ACCEPT)
	{
		//char *filename;
		GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
		filename = gtk_file_chooser_get_filename(chooser);
		//open_file (filename);
		//g_free (filename);

	}
	gtk_widget_destroy(dialog);
}

//Loads game to memory
void loadgame(){
	FILE *space;
	chooseProgram(); //Opens a window to choose ROM
	space = fopen(filename, "rb");
	fseek(space, 0, SEEK_END);
	long bufferSize = ftell(space);
	rewind(space);
	unsigned char * buffer = (unsigned char *)malloc(sizeof(char) * bufferSize);
	size_t result = fread(buffer, 1, bufferSize, space);
	for (int i = 0; i < bufferSize; ++i){ //Load ROM at 0x200
		memory[i + 512] = buffer[i];
	}
	fclose(space);
	free(buffer);
}
#endif