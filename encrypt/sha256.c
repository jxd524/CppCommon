#include "crypt.h"

/* the K array */
static const unsigned long K[64] = {
    0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 0x3956c25bUL,
    0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL, 0xd807aa98UL, 0x12835b01UL,
    0x243185beUL, 0x550c7dc3UL, 0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL,
    0xc19bf174UL, 0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
    0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL, 0x983e5152UL,
    0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 0xc6e00bf3UL, 0xd5a79147UL,
    0x06ca6351UL, 0x14292967UL, 0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL,
    0x53380d13UL, 0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
    0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL, 0xd192e819UL,
    0xd6990624UL, 0xf40e3585UL, 0x106aa070UL, 0x19a4c116UL, 0x1e376c08UL,
    0x2748774cUL, 0x34b0bcb5UL, 0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL,
    0x682e6ff3UL, 0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
    0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

/* Various logical functions */
#define Ch(x,y,z)	((x & y) ^ (~x & z))
#define Maj(x,y,z)  ((x & y) ^ (x & z) ^ (y & z))
#define S(x, n)		(((x)>>((n)&31))|((x)<<(32-((n)&31))))
#define R(x, n)		((x)>>(n))
#define Sigma0(x)	(S(x, 2) ^ S(x, 13) ^ S(x, 22))
#define Sigma1(x)	(S(x, 6) ^ S(x, 11) ^ S(x, 25))
#define Gamma0(x)	(S(x, 7) ^ S(x, 18) ^ R(x, 3))
#define Gamma1(x)	(S(x, 17) ^ S(x, 19) ^ R(x, 10))

/* compress 512-bits */
static void sha256_compress(union hash_state * md)
{
    unsigned long S[8], W[64], t0, t1;
    int i;

    /* copy state into S */
    for (i = 0; i < 8; i++)
        S[i] = md->sha256.state[i];

    /* copy the state into 512-bits into W[0..15] */
    for (i = 0; i < 16; i++)
        W[i] = (((unsigned long) md->sha256.buf[(4 * i) + 0]) << 24) |
            (((unsigned long) md->sha256.buf[(4 * i) + 1]) << 16) |
            (((unsigned long) md->sha256.buf[(4 * i) + 2]) << 8) |
            (((unsigned long) md->sha256.buf[(4 * i) + 3]));

    /* fill W[16..63] */
    for (i = 16; i < 64; i++)
        W[i] = Gamma1(W[i - 2]) + W[i - 7] + Gamma0(W[i - 15]) + W[i - 16];

    /* Compress */
    for (i = 0; i < 64; i++) {
        t0 = S[7] + Sigma1(S[4]) + Ch(S[4], S[5], S[6]) + K[i] + W[i];
        t1 = Sigma0(S[0]) + Maj(S[0], S[1], S[2]);
        S[7] = S[6];
        S[6] = S[5];
        S[5] = S[4];
        S[4] = S[3] + t0;
        S[3] = S[2];
        S[2] = S[1];
        S[1] = S[0];
        S[0] = t0 + t1;
    }

    /* feedback */
    for (i = 0; i < 8; i++)
        md->sha256.state[i] += S[i];
}

/* init the sha256 state */
void sha256_init(union hash_state * md)
{
    md->sha256.curlen = md->sha256.length = 0;
    md->sha256.state[0] = 0x6A09E667UL;
    md->sha256.state[1] = 0xBB67AE85UL;
    md->sha256.state[2] = 0x3C6EF372UL;
    md->sha256.state[3] = 0xA54FF53AUL;
    md->sha256.state[4] = 0x510E527FUL;
    md->sha256.state[5] = 0x9B05688CUL;
    md->sha256.state[6] = 0x1F83D9ABUL;
    md->sha256.state[7] = 0x5BE0CD19UL;
}

void sha256_process(union hash_state * md, unsigned char *buf, int len)
{
    while (len--) {
        /* copy byte */
        md->sha256.buf[md->sha256.curlen++] = *buf++;

        /* is 64 bytes full? */
        if (md->sha256.curlen == 64) {
            sha256_compress(md);
            md->sha256.length += 512;
            md->sha256.curlen = 0;
        }
    }
}

void sha256_done(union hash_state * md, unsigned char *hash)
{
    int i;

    /* increase the length of the message */
    md->sha256.length += md->sha256.curlen * 8;

    /* append the '1' bit */
    md->sha256.buf[md->sha256.curlen++] = 0x80;

    /* if the length is currenlly above 56 bytes we append zeros
                               * then compress.  Then we can fall back to padding zeros and length
                               * encoding like normal.
                             */
    if (md->sha256.curlen >= 56) {
        for (; md->sha256.curlen < 64;)
            md->sha256.buf[md->sha256.curlen++] = 0;
        sha256_compress(md);
        md->sha256.curlen = 0;
    }

    /* pad upto 56 bytes of zeroes */
    for (; md->sha256.curlen < 56;)
        md->sha256.buf[md->sha256.curlen++] = 0;

    /* since all messages are under 2^32 bits we mark the top bits zero */
    for (i = 56; i < 60; i++)
        md->sha256.buf[i] = 0;

    /* append length */
    for (i = 60; i < 64; i++)
        md->sha256.buf[i] = (md->sha256.length >> ((63 - i) * 8)) & 255;
    sha256_compress(md);

    /* copy output */
    for (i = 0; i < 32; i++)
        hash[i] = (md->sha256.state[i >> 2] >> (((3 - i) & 3) << 3)) & 255;
}

int sha256_test(void)
{
  static unsigned char hash[32] = { 
     0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde,
     0x5d, 0xae, 0x22, 0x23, 0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
     0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad };
  static unsigned char *message = "abc";
  unsigned char tmp[32];

  if (hash_memory(find_hash("sha256"), message, strlen(message), tmp) == CRYPT_ERROR) return CRYPT_ERROR;

  if (memcmp(tmp, hash, 32)) { crypt_error = "MD5 hash did not match test vector."; return CRYPT_ERROR; }
  return CRYPT_OK;
}