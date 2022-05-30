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

    assert_int_equal(rs16.mm,       8);
    assert_int_equal(rs16.nn,       0xFF);
//    assert_int_equal(rs16.alpha_to, "1, 2, 4, 8, 10, 20, 40, 80, 87, 89, 95, ad, dd, 3d, 7a, f4");
//    assert_int_equal(rs16.index_of, "ff, 0, 1, 63, 2, c6, 64, 6a, 3, cd, c7, bc, 65, 7e, 6b, 2a");
//    assert_int_equal(rs16.genpoly,  "7a, f0, 12, b4, c7, b5, dd, 31, ea, e1, 3f, c7, 8a, 28, 36, c5");
//    assert_int_equal(rs16.nroots,   0x10);
//    assert_int_equal(rs16.fcr,      70);
//    assert_int_equal(rs16.prim,     0x0B);
//    assert_int_equal(rs16.iprim,    74);
//    assert_int_equal(rs16.pad,      0);
}

static void rsc_encode_test(void **state)
{
//    rsc_encode(rs16, data, par);
}

static void rsc_decode_test(void **state)
{
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
