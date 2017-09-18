#define _XOPEN_SOURCE 
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>    
#include <getopt.h>
#include <time.h>


//#include <myerror.h>
//#include <stdmidi.h>
#define ONCE
#include "midiviterbi.h"

#ifdef undef
#define CON7

#ifdef CON5
#define NN (2)    // Inverse of rate
#define KK (5)    // Constraint length
int polys[] = { 0x13, 0x1b };
#endif

#ifdef CON7
#define NN (2)    // Inverse of rate
#define KK (7)    // Constraint length
int polys[] = { 0x5b, 0x79 };
#endif

#ifdef CON73
#define NN (3)    // Inverse of rate
#define KK (7)    // Constraint length
int polys[] = { 0x5b, 0x79, 0x65 };
#endif

typedef int (*metriccalculator)(int, int *, int *);

int SoftDecisionViterbitDecoder(int N, int K, int polynom[], int L, 
                                int *symbols, int *decoded, int *punctuation,
                                int *metrics, metriccalculator metricfxptr);
int  calcmetricxxx(int n, int *symbol, int *encoderoutput);
double gngauss(double mean, double sigma);

#define VMAXLEN (4*4096)


int main(int narg, char *argp[])
{
    int i, j;
    int punctuation[MAXLEN] = { 1, 1, 1, 0, 1, 1, -1 };
    int isf, facbits;

    char infile[MAXLEN], line[MAXLEN];
    FILE *inf;
    int s, t, q1, cpi, ilv[512], symbols;
    int facsymbols[VMAXLEN], *rxp;
    int decoded[VMAXLEN], metric[VMAXLEN];
    int prbs, ixor, crc, ob;
    
    isf = 0;

    for(isf=0; isf<3; isf++)
    {
    sprintf(infile, "FACbits%d.dat", isf);
    inf = fopen(infile, "r");
    if (inf == NULL)
        Error(ERRFATAL, "could not open file\n");
    fgets(line, MAXLEN-1, inf);
    fclose(inf);

    facbits = strlen(line)-1;
    printf("%d\n", facbits);
    
    s = 2;
    while(s < facbits)
        s *= 2;
    
    t = 21;
    q1 = s/4-1;
    cpi = 0;
    for(i = 0; i < facbits; i++)
    {
        if (i == 0)
            cpi = 0;
        else
        {
            cpi = (t * cpi + q1) % s;
            while(cpi >= facbits)
                    cpi = (t * cpi + q1) % s;
        }
        ilv[cpi] = i;
    }
    
    for(i = 0; i < facbits; i++)
        facsymbols[i] = line[ilv[i]] == '1' ? -128 : 128;
    
    symbols = (3*facbits)/5;

    SoftDecisionViterbitDecoder(NN, KK, polys, symbols, facsymbols, decoded, 
                                punctuation, metric, calcmetricxxx);
    
    prbs = 0xffff;
    crc  = (1<<8)-1;
    for(i=0;i<symbols; i++)
    {
        ixor = ((prbs & 0x10) != 0) != ((prbs & 0x100) != 0);
        prbs <<= 1;
        prbs |= ixor;
        decoded[i] = decoded[i] != ixor;
        if (i<64)
        {
            ob   = (crc & 0x01) != 0;
            ob   = decoded[i] != ob;
            crc ^= ob ? 0x70 : 0x00;
            crc >>= 1;
            crc |= ob ? 0x80 : 0x00;
        }
    }

    
    for(i=0; i<symbols; i++)
        printf("%d", decoded[i]);
    printf("\n");

    for(i=0; i<64; i++)
        printf(" ");

    for(i=0; i<8; i++)
        printf("%d", (crc & (1<<i)) == 0);
    printf("\n");

    }


}


int main1(int narg, char *argp[])
{
    int i, j, symbols, state, bit, out[10], optc;
    int invec[VMAXLEN], decoded[VMAXLEN];
    int rxsymbol[VMAXLEN], *rxp;
    int metric[VMAXLEN];
    int fi, ipu;
    double ddummy, noise;
    int punctuation[MAXLEN] = { 1, 1, 1, 0, 1, 1, -1 };

    noise = 0;

    while ((optc = getopt(narg, argp, "hn:")) > 0) 
    {
        switch (optc)
        { 
          /* Help */
          case 'h':
//            fprintf(stderr, "%s", HelpText);
            exit(0);
            break;
          case 'n':
            ddummy = 0;
            fi = sscanf(optarg, "%lf", &ddummy);
            if (fi < 1)
                Error(ERRFATAL, "illegal noise");
            noise = ddummy;
            break;
          default:
            break;
        }
    }
    

    symbols = 1000;
    state   = 0;
    rxp = rxsymbol;
    ipu = 0;
    for(i=0; i<symbols; i++)
    {
        bit = rand() % 2;
        invec[i] = bit;
        state |= bit ? 1 << (KK-1) : 0;
        for(j=0; j<NN; j++)
            out[j] = BitsinByte[state & polys[j]] % 2;
        state >>= 1;
        
        for(j=0; j<NN; j++)
        {
            if (ipu != 3)
                *rxp++ = 128-256*out[j] + int(gngauss(0.0, noise));
            ipu = (ipu+1) % 6;
        }
    }


    /*
     * -------------------------
     */

    SoftDecisionViterbitDecoder(NN, KK, polys, symbols, (int*)rxsymbol, decoded, 
                                punctuation, metric, calcmetricxxx);

    int metmean, metlow, metnextlow, err;

    metmean = 0;
    metlow = metric[0];
    for(i=0; i < 1 << (KK-1); i++)
    {
        metmean += metric[i];
        if (metlow > metric[i])
            metlow = metric[i];
    }
    metmean /= 1 << (KK-1);

    metnextlow = 10000000;
    for(i=0; i < 1 << (KK-1); i++)
    {
        if (metnextlow > metric[i] && metric[i] !=metlow)
            metnextlow = metric[i];
    }

    printf("%8d %8d %8d\n", metmean, metnextlow, metlow);
    
    err = 0;
    for(i=0; i<symbols; i++)
        if (invec[i] != decoded[i])
            err++;

    printf("%8d %8d\n", symbols, err);
    
}

int  calcmetricxxx(int n, int *symbol, int *encoderoutput)
{
    int i, d, dtot;
    dtot = 0;
    for(i = 0; i < n; i++)
    {
        d = symbol[i] - (128-256*encoderoutput[i]);
        if (d > 0)
            dtot += d;
        else
            dtot -= d;
    }
    return dtot;
}

#endif


#define ZERODISTANCE             (0)
#define VERYHIGHDISTANCE  (10000000)
#define NMAX (10)

int SoftDecisionViterbitDecoder(int N, int K, const int polynom[], int L,
							    int8_t  *symbols, int *decoded, const int *punctuation,
                                int *metrics, metriccalculator metricfxptr)
{
    int M, S;
    int state  ;
    int *accA, *accB, *output0, *output1, *maxvector, *statehistory;
    int *presentaccptr, *newaccptr, *tmpaccptr;
    int outputpattern, isym, n;
    int dist0, dist1, next0, next1, accdist0, accdist1; 
    int lowmetric, lowmetricstate, imax=0;
    int inbitpos, is, ipunct;
	int8_t locsym[NMAX];
    int8_t *isp;

    M = K-1;
    S = 1 << M;
    inbitpos = 1 << (M-1);
    
    accA         = (int*)malloc(S * sizeof(int));
    accB         = (int*)malloc(S * sizeof(int));
    output0      = (int*)malloc(S * N * sizeof(int));
    output1      = (int*)malloc(S * N * sizeof(int));
    maxvector    = (int*)malloc(S * sizeof(int));
    statehistory = (int*)malloc(S * (L+M) * sizeof(int));

    if (accA      == NULL || accB         == NULL || 
        output0   == NULL || output1      == NULL || 
        maxvector == NULL || statehistory == NULL  )
        return -1;
    
    for(state = 0; state < S; state++)
    {
        for(n=0; n<N; n++)
        {
            output0[state*N + n] = ViterbiBitsinByte[state & polynom[n]] % 2;
            output1[state*N + n] = !output0[state*N + n];
        }
        maxvector[state] = VERYHIGHDISTANCE;
    }

    presentaccptr = accA;
    newaccptr     = accB;

    memcpy(newaccptr,     maxvector, S*sizeof(int));
    memcpy(presentaccptr, maxvector, S*sizeof(int));
    presentaccptr[0] = ZERODISTANCE;

    isp = symbols;
    ipunct = 0;

    for(isym = 0; isym < L + M; isym++)
    {
        if (punctuation == NULL)
        {
            memcpy(locsym, isp, sizeof(int8_t)*N);
            isp += N;
        }
        else
            for(is=0; is<N; is++)
            {
                if (punctuation[ipunct] == -1)
                    ipunct = 0;
                if (punctuation[ipunct] ==  0)
                    locsym[is] = 0;
                else
                     locsym[is] = *isp++;
                ipunct++;
                imax++;
            }

        memcpy(newaccptr, maxvector, S*sizeof(int));
        for(state = 0; state < S; state++)
        {
            if (isym >= L)
            {
                if (isym == L && metrics != NULL)
                    memcpy(metrics, presentaccptr, S*sizeof(int));

                dist0    = ZERODISTANCE;
                dist1    = VERYHIGHDISTANCE;
            }
            else
            {
                dist0    = metricfxptr(N, locsym,  &(output0[state*N]));
                dist1    = metricfxptr(N, locsym,  &(output1[state*N]));
            }
             
            next0    = (state >> 1);
            next1    = next0 | inbitpos;

            accdist0 = presentaccptr[state] + dist0;
            accdist1 = presentaccptr[state] + dist1;
            
            if (newaccptr[next0] > accdist0)
            {
                newaccptr[next0] = accdist0;
                statehistory[next0 + isym*S] = state;
            }
            if (newaccptr[next1] > accdist1)
            {
                newaccptr[next1] = accdist1;
                statehistory[next1 + isym*S] = state;
            }
        }
        tmpaccptr     = newaccptr;
        newaccptr     = presentaccptr;
        presentaccptr = tmpaccptr;
    }

    
    
    lowmetricstate = 0;
    lowmetric      = presentaccptr[lowmetricstate];

    for(state = 0; state < S; state++)
        if (presentaccptr[state] < lowmetric)
        {
            lowmetric = presentaccptr[state];
            lowmetricstate = state; 
        }
    
    state = lowmetricstate;

    for(isym = L+M-1; isym >= 1; isym--)
    {
        state = statehistory[state + isym*S];
        if (isym <= L)
            decoded[isym-1] = (state & inbitpos) != 0;
    }


    free(accA);        
    free(accB);        
    free(output0);
    free(output1);
    free(maxvector);
    free(statehistory);

    return L;
}

#define LRAND_MAX 100

double gngauss(double mean, double sigma) 
{
    /* This uses the fact that a Rayleigh-distributed random variable R, with
    the probability distribution F(R) = 0 if R < 0 and F(R) =
    1 - exp(-R^2/2*sigma^2) if R >= 0, is related to a pair of Gaussian
    variables C and D through the transformation C = R * cos(theta) and
    D = R * sin(theta), where theta is a uniformly distributed variable
    in the interval (0, 2*pi()). From Contemporary Communication Systems
    USING MATLAB(R), by John G. Proakis and Masoud Salehi, published by
    PWS Publishing Company, 1998, pp 49-50. This is a pretty good book. */
 
    double u, r;            /* uniform and Rayleigh random variables */
 
    /* generate a uniformly distributed random number u between 0 and 1 - 1E-6*/
    u = (double)rand()/RAND_MAX;
    if (u > 1.0-1e-15) u = 0.999999999;
 
    /* generate a Rayleigh-distributed random number r using u */
    r = sigma * sqrt( 2.0 * log( 1.0 / (1.0 - u) ) );
 
    /* generate another uniformly-distributed random number u as before*/
    u = (double)rand()/RAND_MAX;
    if (u == 1.0) u = 0.999999999;
 
    /* generate and return a Gaussian-distributed random number using r and u */
    return( (double) ( mean + r * cos(2 * M_PI * u) ) );
}

