#include <iostream>
#include <lgpio.h>
#include <thread>

int main()
{
    int h = lgGpiochipOpen(0);
    if (h < 0)
        return 1;

    const int PIN = 18;
    if (lgGpioClaimOutput(h, 0, PIN, 0) == LG_OKAY)
    {

        std::cout << lgTxPwm(h, PIN, 1,  50,  7.5, 1) << std::endl;; 
        std::this_thread::sleep_for(std::chrono::seconds(7));
        // Rate is controlled by the 5th parameter (duty cycle)
        lgTxPwm(h, PIN, 1,  50, 7.0, 0);
        lguSleep(5);
        lgTxPwm(h, PIN, 0, 0, 0, 0);
    }
    lgGpiochipClose(h);
    return 0;
}
