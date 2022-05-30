/*
 * test.c
 * 
 * Copyright The RSCLib Contributors.
 * 
 * This file is part of RSCLib.
 * 
 * RSCLib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * RSCLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with RSCLib. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

/**
 * \brief Reed-Solomon C library unit test.
 * 
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * 
 * \version 0.0.1
 * 
 * \date 2022/05/30
 * 
 * \defgroup test Test
 * \ingroup rsclib
 * \{
 */

#include <stdio.h>

#include <rsc/rsc.h>

int main()
{
    reed_solomon_t rs16 = {0};

    if (rsc_init(8, 0x187, 112, 11, 16, 0, &rs16) != 0)
    {
        printf("Error initializing RS codec!\n\r");

        return -1;
    }

    uint8_t i = 0;

    printf("MM: %x\n\r", rs16.mm);
    printf("NN: %x\n\r", rs16.nn);
    for(i = 0; i < 16; i++)
    {
        printf("%x, ", rs16.alpha_to[i]);
    }
    printf("\n\r");
    for(i = 0; i < 16; i++)
    {
        printf("%x, ", rs16.index_of[i]);
    }
    printf("\n\r");
    for(i = 0; i < 16; i++)
    {
        printf("%x, ", rs16.genpoly[i]);
    }
    printf("\n\r");
    printf("nroots: %x\n\r", rs16.nroots);
    printf("FCR: %x\n\r", rs16.fcr);
    printf("prim: %x\n\r", rs16.prim);
    printf("iprim: %x\n\r", rs16.iprim);
    printf("pad: %x\n\r", rs16.pad);

    uint8_t data[32] = {0};
    uint8_t par[16] = {0};

    for(i = 0; i < 32; i++)
    {
        data[i] = i;
    }

    rsc_encode(rs16, data, par);

    for(i = 0; i < 16; i++)
    {
        printf("%x, ", par[i]);
    }
    printf("\n\r");

    return 0;
}

/**< \} End of test group */
