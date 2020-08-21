/* File encoding format:UTF-8  */

#include "currency.h"
#include <errno.h>

uint16_t errnum = ERR_SUCCESS;

void perrnum(const char *s)
{
    printf("%s", s);
    if (errnum == ERR_SUCCESS)
    {
        printf("\nsuccess!\n");
    }
    else if (errnum == ERR_PARAMETER)
    {
        printf("\nError in transmitting parameters!\n");
    }
    else if (errnum == ERR_NULL_POINTER)
    {
        printf("\nA null pointer was passed in!\n");
    }
    else if (errnum == ERR_ADDRESS_FALSE)
    {
        printf("\nAddress error, please check whether there is this address or spelling error!\n");
    }
    else if (errnum == ERR_SRAND_FLASE)
    {
        perror("\nUnable to open seed!");
    }
}
