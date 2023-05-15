/*
 * rsc.c
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
 * \brief Reed-Solomon C library implementation.
 * 
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * 
 * \version 0.0.5
 * 
 * \date 2022/03/06
 * 
 * \addtogroup rsclib
 * \{
 */

#include <string.h>

#include <rsc/rsc.h>

#define	MIN(a, b)   ((a) < (b) ? (a) : (b))

/**
 * \brief .
 *
 * \param[in] rs .
 *
 * \param[in] x .
 *
 * \return .
 */
int modnn(reed_solomon_t *rs, int x);

int rsc_init(int symsize, int gfpoly, int fcr, int prim, int nroots, int pad, reed_solomon_t *rs)
{
    int err = 0;

    int i, j, sr, root, iprim;

    /* Check parameter ranges */
    if ((symsize >= 0) && (symsize <= (8 * sizeof(uint8_t))))
    {
        if ((fcr >= 0) && (fcr < (1 << symsize)))
        {
            if ((prim > 0) && (prim < (1 << symsize)))
            {
                if ((nroots >= 0) && (nroots < (1 << symsize)))
                {
                    if ((pad >= 0) && (pad < ((1 << symsize) - 1 - nroots)))
                    {
                        rs->mm = symsize;
                        rs->nn = (1 << symsize) - 1;
                        rs->pad = pad;

                        /* Generate Galois field lookup tables */
                        rs->index_of[0] = rs->nn;   /* log(zero) = -inf */
                        rs->alpha_to[rs->nn] = 0;   /* alpha**-inf = 0 */
                        sr = 1;
                        for(i = 0; i < rs->nn; i++)
                        {
                            rs->index_of[sr] = i;
                            rs->alpha_to[i] = sr;
                            sr <<= 1;
                            if (sr & (1 << symsize))
                            {
                                sr ^= gfpoly;
                            }
                            sr &= rs->nn;
                        }
                        if (sr == 1)
                        {
                            rs->fcr = fcr;
                            rs->prim = prim;
                            rs->nroots = nroots;

                            /* Find prim-th root of 1, used in decoding */
                            for(iprim = 1; (iprim % prim) != 0; iprim += rs->nn)
                            {
                            }
                            rs->iprim = iprim / prim;

                            rs->genpoly[0] = 1;
                            for(i = 0, root = fcr * prim; i < nroots; i++, root += prim)
                            {
                                rs->genpoly[i + 1] = 1;

                                /* Multiply rs->genpoly[] by  @**(root + x) */
                                for(j = i; j > 0; j--)
                                {
                                    if (rs->genpoly[j] != 0)
                                    {
                                        rs->genpoly[j] = rs->genpoly[j - 1] ^ rs->alpha_to[modnn(rs, rs->index_of[rs->genpoly[j]] + root)];
                                    }
                                    else
                                    {
                                        rs->genpoly[j] = rs->genpoly[j - 1];
                                    }
                                }
                                /* rs->genpoly[0] can never be zero */
                                rs->genpoly[0] = rs->alpha_to[modnn(rs, rs->index_of[rs->genpoly[0]] + root)];
                            }
                            /* convert rs->genpoly[] to index form for quicker encoding */
                            for(i = 0; i <= nroots; i++)
                            {
                                rs->genpoly[i] = rs->index_of[rs->genpoly[i]];
                            }

                            err = 0;
                        }
                        else
                        {
                            /* field generator polynomial is not primitive! */
                            memset(rs, 0, sizeof(reed_solomon_t));

                            err = -1;
                        }
                    }
                    else
                    {
                        err = 0;    /* Too much padding */
                    }
                }
                else
                {
                    err = -1;   /* Can't have more roots than symbol values! */
                }
            }
            else
            {
                err = -1;
            }
        }
        else
        {
            err = -1;
        }
    }
    else
    {
        err = -1;
    }

    return err;
}

void rsc_encode(reed_solomon_t *rs, uint8_t *data, uint8_t *parity, uint8_t *parity_len)
{
    int i = 0;
    int j = 0;
    int feedback = 0;

    memset(parity, 0, rs->nroots);

    for(i = 0; i < (rs->nn - rs->nroots - rs->pad); i++)
    {
        feedback = rs->index_of[data[i] ^ parity[0]];
        if (feedback != rs->nn)  /* feedback term is non-zero */
        {
            for(j = 0; j < rs->nroots; j++)
            {
	            parity[j] ^= rs->alpha_to[modnn(rs, feedback + rs->genpoly[rs->nroots - j])];
            }
        }
        /* Shift */
        memmove(&parity[0], &parity[1], rs->nroots - 1);
        if (feedback != rs->nn)
        {
            parity[rs->nroots - 1] = rs->alpha_to[modnn(rs, feedback + rs->genpoly[0])];
        }
        else
        {
            parity[rs->nroots - 1] = 0U;
        }
    }

    *parity_len = rs->nroots;
}

int rsc_decode(reed_solomon_t *rs, uint8_t *data, int *eras_pos, int no_eras)
{
    int retval = -1;

    int deg_lambda, el, deg_omega;
    int i, j, r, k;
    uint8_t u, q, tmp, num1, num2, den, discr_r;
    uint8_t lambda[rs->nroots + 1], s[rs->nroots];  /* Err+Eras Locator poly and syndrome poly */
    uint8_t b[rs->nroots + 1], t[rs->nroots + 1], omega[rs->nroots + 1];
    uint8_t root[rs->nroots], reg[rs->nroots + 1], loc[rs->nroots];
    int syn_error, count;

    /* form the syndromes; i.e., evaluate data(x) at roots of g(x) */
    for(i = 0; i < rs->nroots; i++)
    {
        s[i] = data[0];
    }

    for(j = 1; j < rs->nn - rs->pad; j++)
    {
        for(i = 0; i < rs->nroots; i++)
        {
            if (s[i] == 0)
            {
                s[i] = data[j];
            }
            else
            {
                s[i] = data[j] ^ rs->alpha_to[modnn(rs, rs->index_of[s[i]] + (rs->fcr + i) * rs->prim)];
            }
        }
    }

    /* Convert syndromes to index form, checking for nonzero condition */
    syn_error = 0;
    for(i = 0; i < rs->nroots; i++)
    {
        syn_error |= s[i];
        s[i] = rs->index_of[s[i]];
    }

    if (!syn_error)
    {
        /* if syndrome is zero, data[] is a codeword and there are no errors to correct. So return data[] unmodified */
        count = 0;
        goto finish;
    }
    memset(&lambda[1], 0, rs->nroots * sizeof(lambda[0]));
    lambda[0] = 1;

    if (no_eras > 0)
    {
        /* Init lambda to be the erasure locator polynomial */
        lambda[1] = rs->alpha_to[modnn(rs, rs->prim * (rs->nn - 1 - eras_pos[0]))];
        for(i = 1; i < no_eras; i++)
        {
            u = modnn(rs, rs->prim * (rs->nn - 1 - eras_pos[i]));
            for(j = i + 1; j > 0; j--)
            {
                tmp = rs->index_of[lambda[j - 1]];
                if (tmp != rs->nn)
                {
                    lambda[j] ^= rs->alpha_to[modnn(rs, u + tmp)];
                }
            }
        }
    }
    for(i = 0; i < rs->nroots + 1; i++)
    {
        b[i] = rs->index_of[lambda[i]];
    }

    /* Begin Berlekamp-Massey algorithm to determine error+erasure locator polynomial */
    r = no_eras;
    el = no_eras;
    while(++r <= rs->nroots)    /* r is the step number */
    {
        /* Compute discrepancy at the r-th step in poly-form */
        discr_r = 0;
        for(i = 0; i < r; i++)
        {
            if ((lambda[i] != 0) && (s[r - i - 1] != rs->nn))
            {
                discr_r ^= rs->alpha_to[modnn(rs, rs->index_of[lambda[i]] + s[r - i - 1])];
            }
        }
        discr_r = rs->index_of[discr_r];    /* Index form */
        if (discr_r == rs->nn)
        {
            /* 2 lines below: B(x) <-- x*B(x) */
            memmove(&b[1], b, rs->nroots * sizeof(b[0]));
            b[0] = rs->nn;
        }
        else
        {
            /* 7 lines below: T(x) <-- lambda(x) - discr_r*x*b(x) */
            t[0] = lambda[0];
            for(i = 0 ; i < rs->nroots; i++)
            {
                if(b[i] != rs->nn)
                {
                    t[i+1] = lambda[i+1] ^ rs->alpha_to[modnn(rs, discr_r + b[i])];
                }
                else
                {
                    t[i+1] = lambda[i+1];
                }
            }
            if (2 * el <= r + no_eras - 1)
            {
                el = r + no_eras - el;
                /* 2 lines below: B(x) <-- inv(discr_r) * lambda(x) */
                for(i = 0; i <= rs->nroots; i++)
                {
                    b[i] = (lambda[i] == 0) ? rs->nn : modnn(rs, rs->index_of[lambda[i]] - discr_r + rs->nn);
                }
            }
            else
            {
                /* 2 lines below: B(x) <-- x*B(x) */
                memmove(&b[1], b, rs->nroots * sizeof(b[0]));
                b[0] = rs->nn;
            }
            memcpy(lambda, t, (rs->nroots + 1) * sizeof(t[0]));
        }
    }

    /* Convert lambda to index form and compute deg(lambda(x)) */
    deg_lambda = 0;
    for(i = 0; i < rs->nroots + 1; i++)
    {
        lambda[i] = rs->index_of[lambda[i]];
        if (lambda[i] != rs->nn)
        {
            deg_lambda = i;
        }
    }
    /* Find roots of the error+erasure locator polynomial by Chien search */
    memcpy(&reg[1], &lambda[1], rs->nroots * sizeof(reg[0]));
    count = 0;  /* Number of roots of lambda(x) */
    for(i = 1, k = rs->iprim - 1; i <= rs->nn; i++, k = modnn(rs, k + rs->iprim))
    {
        q = 1;  /* lambda[0] is always 0 */
        for(j = deg_lambda; j > 0; j--)
        {
            if (reg[j] != rs->nn)
            {
                reg[j] = modnn(rs, reg[j] + j);
                q ^= rs->alpha_to[reg[j]];
            }
        }
        if (q != 0)
        {
            continue;   /* Not a root */
        }
        /* store root (index-form) and error location number */
        root[count] = i;
        loc[count] = k;
        /* If we've already found max possible roots, abort the search to save time */
        if (++count == deg_lambda)
        {
            break;
        }
    }
    if (deg_lambda != count)
    {
        /* deg(lambda) unequal to number of roots => uncorrectable error detected */
        count = -1;
        goto finish;
    }
    /* Compute err+eras evaluator poly omega(x) = s(x)*lambda(x) (modulo x**rs.nroots). in index form. Also find deg(omega). */
    deg_omega = deg_lambda-1;
    for(i = 0; i <= deg_omega; i++)
    {
        tmp = 0;
        for(j = i; j >= 0; j--)
        {
            if ((s[i - j] != rs->nn) && (lambda[j] != rs->nn))
            {
                tmp ^= rs->alpha_to[modnn(rs, s[i - j] + lambda[j])];
            }
        }
        omega[i] = rs->index_of[tmp];
    }

    /* Compute error values in poly-form. num1 = omega(inv(X(l))), num2 = inv(X(l))**(rs.fcr-1) and den = lambda_pr(inv(X(l))) all in poly-form */
    for(j = count-1; j >=0; j--)
    {
        num1 = 0;
        for(i = deg_omega; i >= 0; i--)
        {
            if (omega[i] != rs->nn)
            {
                num1 ^= rs->alpha_to[modnn(rs, omega[i] + i * root[j])];
            }
        }
        num2 = rs->alpha_to[modnn(rs, root[j] * (rs->fcr - 1) + rs->nn)];
        den = 0;

        /* lambda[i+1] for i even is the formal derivative lambda_pr of lambda[i] */
        for (i = MIN(deg_lambda, rs->nroots - 1) & ~1; i >= 0; i -=2)
        {
            if (lambda[i + 1] != rs->nn)
            {
                den ^= rs->alpha_to[modnn(rs, lambda[i + 1] + i * root[j])];
            }
        }
        /* Apply error to data */
        if (num1 != 0 && loc[j] >= rs->pad)
        {
            data[loc[j] - rs->pad] ^= rs->alpha_to[modnn(rs, rs->index_of[num1] + rs->index_of[num2] + rs->nn - rs->index_of[den])];
        }
    }

finish:

    if (eras_pos != NULL)
    {
        for(i = 0; i < count; i++)
        {
            eras_pos[i] = loc[i];
        }
    }
    retval = count;

    return retval;
}

int modnn(reed_solomon_t *rs, int x)
{
    while(x >= rs->nn)
    {
        x -= rs->nn;
        x = (x >> rs->mm) + (x & rs->nn);
    }

    return x;
}

/**< \} End of rsclib group */
