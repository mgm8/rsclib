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
 * \version 0.0.2
 * 
 * \date 2022/05/30
 * 
 * \defgroup test Test
 * \ingroup rsclib
 * \{
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <float.h>
#include <cmocka.h>

#include <rsc/rsc.h>

static void rsc_init_test(void **state)
{
    reed_solomon_t rs16 = {0};

    assert_return_code(rsc_init(8, 0x187, 112, 11, 16, 0, &rs16), 0);

    /* Expected results */
    uint8_t alpha_to[50] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x87, 0x89, 0x95, 0xAD, 0xDD, 0x3D, 0x7A, 0xF4};
    uint8_t index_of[50] = {0xFF, 0x00, 0x01, 0x63, 0x02, 0xC6, 0x64, 0x6A, 0x03, 0xCD, 0xC7, 0xBC, 0x65, 0x7E, 0x6B, 0x2A};
    uint8_t genpoly[50] = {0x7A, 0xF0, 0x12, 0xB4, 0xC7, 0xB5, 0xDD, 0x31, 0xEA, 0xE1, 0x3F, 0xC7, 0x8A, 0x28, 0x36, 0xC5};

    assert_int_equal(rs16.mm,       8);
    assert_int_equal(rs16.nn,       0xFF);
    assert_memory_equal(rs16.alpha_to, alpha_to, 16);
    assert_memory_equal(rs16.index_of, index_of, 16);
    assert_memory_equal(rs16.genpoly, genpoly, 16);
    assert_int_equal(rs16.nroots,   0x10);
    assert_int_equal(rs16.fcr,      70);
    assert_int_equal(rs16.prim,     0x0B);
    assert_int_equal(rs16.iprim,    74);
    assert_int_equal(rs16.pad,      0);
}

static void rsc_encode_test(void **state)
{
    /* Expected result */
    uint8_t par_ref[32] = {0x59, 0x93, 0xFB, 0xBC, 0xF5, 0xE6, 0xC2, 0x90, 0xD0, 0x6E, 0x77, 0xCB, 0x83, 0xAA, 0xC8, 0xEE};

    reed_solomon_t rs16 = {0};

    rsc_init(8, 0x187, 112, 11, 16, 0, &rs16);

    uint8_t data[220] = {0};
    uint8_t par[32] = {0};

    rsc_encode(rs16, data, par);

    assert_memory_equal(par_ref, par, 16);
}

static void rsc_decode_test(void **state)
{
    reed_solomon_t rs16 = {0};

    rsc_init(8, 0x187, 112, 11, 16, 0, &rs16);

    uint8_t data[220] = {0};
    uint8_t par[32] = {0};
    uint8_t pkt[300] = {0};

//    rsc_encode(rs16, data, par);
//
//    uint8_t eras_pos[32] = {0};
//
//    assert_return_code(rsc_decode(rs16, pkt, eras_pos, 0), 0);
}

int main()
{
    const struct CMUnitTest rsc_tests[] = {
        cmocka_unit_test(rsc_init_test),
        cmocka_unit_test(rsc_encode_test),
        cmocka_unit_test(rsc_decode_test),
    };

    return cmocka_run_group_tests(rsc_tests, NULL, NULL);
}

/**< \} End of test group */
