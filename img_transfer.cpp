#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>  
#include <string>
#include <afx.h>
#include <iostream>
#include "unistd.h"
#include <Windows.h>
#include <filesystem>
#include <lmcons.h>
using namespace std;

void RefreshDirectory(LPTSTR);
void RefreshTree(LPTSTR);
void WatchDirectory(LPTSTR);
char* currentDir_string();

char uname[257];


void _tmain(int argc, TCHAR* argv[])
{
    char* cwd = currentDir_string();
    argv[1] = cwd;
    DWORD size = 257;
    GetUserName(uname, &size);

    //if (argc != 2)
    //{
    //    _tprintf(TEXT("Usage: %s <dir>\n"), argv[0]);
    //    return;
    //}

    cout << "==================== OZ Photobooth v01 Image Transfer ====================" << endl;
    cout << "Release Notes:\n13-03-2021 C++ codes used to build this console application are not proprietary and \nwere sourced from different websites.\n" << endl;
    cout << "We'd like to thank the following for their code snippets: \n\t- Brian Gianforcano \n\t- Microsoft \n\t- Elektrik Adam\n" << endl;
    cout << "Current Working Directory: " << cwd << endl;
    cout << "Current User: " << uname << endl;

    while (TRUE) {
        WatchDirectory(argv[1]);
    }
}

char * currentDir_string() {
    char * cwd;
    cwd = (char*)malloc(_MAX_PATH * sizeof(char));
    getcwd(cwd,_MAX_PATH);

    return cwd;
}

string newFileFindPath(string x)
{
    FILETIME bestDate = { 0, 0 };
    FILETIME curDate;
    string name = x + "\\*.jpg";
    const char* c = name.c_str();
    CFileFind finder;

    BOOL bWorking = finder.FindFile(c);

    while (bWorking)
    {

        bWorking = finder.FindNextFile();

        // date = (((ULONGLONG) finder.GetCreationTime(ft).dwHighDateTime) << 32) + finder.GetCreationTime(ft).dwLowDateTime;

        finder.GetCreationTime(&curDate);

        if (CompareFileTime(&curDate, &bestDate) > 0)
        {
            bestDate = curDate;
            name = finder.GetFileName().GetString();
            // name = (LPCTSTR) finder.GetFileName();
        }

    }
    return name;
}

void WatchDirectory(LPTSTR lpDir)
{
    DWORD dwWaitStatus;
    HANDLE dwChangeHandles[2];
    TCHAR lpDrive[4];
    TCHAR lpFile[_MAX_FNAME];
    TCHAR lpExt[_MAX_EXT];
    int newFile = 0; //Flag for when there is a new file
    int newImg_counter = 0; //Counter for file name

    _tsplitpath_s(lpDir, lpDrive, 4, NULL, 0, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);

    lpDrive[2] = (TCHAR)'\\';
    lpDrive[3] = (TCHAR)'\0';

    // Watch the directory for file creation and deletion. 

    dwChangeHandles[0] = FindFirstChangeNotification(
        lpDir,                         // directory to watch 
        FALSE,                         // do not watch subtree 
        FILE_NOTIFY_CHANGE_FILE_NAME); // watch file name changes 

    if (dwChangeHandles[0] == INVALID_HANDLE_VALUE)
    {
        printf("\n ERROR: FindFirstChangeNotification function failed.\n");
        ExitProcess(GetLastError());
    }

    // Watch the subtree for directory creation and deletion. 

    dwChangeHandles[1] = FindFirstChangeNotification(
        lpDrive,                       // directory to watch 
        TRUE,                          // watch the subtree 
        FILE_NOTIFY_CHANGE_DIR_NAME);  // watch dir name changes 

    if (dwChangeHandles[1] == INVALID_HANDLE_VALUE)
    {
        printf("\n ERROR: FindFirstChangeNotification function failed.\n");
        ExitProcess(GetLastError());
    }


    // Make a final validation check on our handles.

    if ((dwChangeHandles[0] == NULL) || (dwChangeHandles[1] == NULL))
    {
        printf("\n ERROR: Unexpected NULL from FindFirstChangeNotification.\n");
        ExitProcess(GetLastError());
    }

    // Change notification is set. Now wait on both notification 
    // handles and refresh accordingly. 

    while (TRUE)
    {
        // Wait for notification.

        printf("\nWaiting for notification...\n");

        dwWaitStatus = WaitForMultipleObjects(2, dwChangeHandles,
            FALSE, INFINITE);

        switch (dwWaitStatus)
        {
        case WAIT_OBJECT_0:

            // A file was created, renamed, or deleted in the directory.
            // Refresh this directory and restart the notification.
            newFile = 1; //Set flag that a new object was found
            
            if (FindNextChangeNotification(dwChangeHandles[0]) == FALSE)
            {
                printf("\n ERROR: FindNextChangeNotification function failed.\n");
                ExitProcess(GetLastError());
            }

            if (newFile == 1) {
                //Subroutine here
                //Get path for new image added to the directory
                string fImgIn = newFileFindPath(lpDir);
                string fImgOutPath = "C:\\Users\\";
                fImgOutPath += uname; 
                fImgOutPath += "\\Desktop\\temp";

                //Debug
                cout << fImgOutPath << endl;

                ////Create new file name for new jpg
                //string fImgName = "img";
                //fImgName += newImg_counter;
                //fImgName += ".jpg";

                string cmdLine = "copy " + fImgIn + " " + fImgOutPath;

                const char* c = cmdLine.c_str(); // Create a command line that will be passed to system

                cout << "Copying " << fImgIn << "..." << endl; //For debug
                system(c); // Call system and execute command line function

                RefreshDirectory(lpDir); //Call to refresh directory after image has been saved
                newFile = 0; //Reset flag
                newImg_counter++; //Increment?
            }

            while (WAIT_OBJECT_0) { // Use this to make sure that the output on CMD doesnt keep repeating
                Sleep(2000);
            }

            break;

        case WAIT_OBJECT_0 + 1:

            // A directory was created, renamed, or deleted.
            // Refresh the tree and restart the notification.

            RefreshTree(lpDrive);
            if (FindNextChangeNotification(dwChangeHandles[1]) == FALSE)
            {
                printf("\n ERROR: FindNextChangeNotification function failed.\n");
                ExitProcess(GetLastError());
            }
            break;

        case WAIT_TIMEOUT:

            // A timeout occurred, this would happen if some value other 
            // than INFINITE is used in the Wait call and no changes occur.
            // In a single-threaded environment you might not want an
            // INFINITE wait.

            printf("\nNo changes in the timeout period.\n");
            break;

        default:
            printf("\n ERROR: Unhandled dwWaitStatus.\n");
            ExitProcess(GetLastError());
            break;
        }

        
    }
}

void RefreshDirectory(LPTSTR lpDir)
{
    // This is where you might place code to refresh your
    // directory listing, but not the subtree because it
    // would not be necessary.
    _tprintf(TEXT("Directory (%s) changed.\n"), lpDir);
}

void RefreshTree(LPTSTR lpDrive)
{
    // This is where you might place code to refresh your
    // directory listing, including the subtree.

    _tprintf(TEXT("Directory tree (%s) changed.\n"), lpDrive);
}