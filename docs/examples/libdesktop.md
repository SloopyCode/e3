# libdesktop
example code:

```c
#include <libdesktop.h>
#include <stdio.h>

int main()
{
    if (!dt_check_abi()) // newest lib version, compatibility
    {
        ...
        _exit(1);
    }

    int winW;
    int winH;
    #define APP_TITLE "test"

    desktopWindowSizeForContent
    (
        320 * 2,  //content w
        200 * 2,  //content h
        DT_WIN,   // style
        &winW,
        &winH
    );

    int AppWindow = desktop.createWindow
    (
        APP_TITLE,
    
        100,   100, // x, y
        winW, 
        winH,
        
        DT_WIN    /* style
                    WIN, POPUP, NOMOVE, NOTITLE */
    );

    // own renderer stuff here
    // or ui16 implementation...
}
```