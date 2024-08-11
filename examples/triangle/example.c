#include <arline.h>
#include <windows.h>

int _fltused;

int main()
{
    ArApplicationInfo applicationInfo;
    applicationInfo.width = 1280;
    applicationInfo.height = 720;
    applicationInfo.enableVsync = true;

    arExecute(&applicationInfo);
    ExitProcess(0);
}