/* Copyright (C) 2018 SCARV project <info@scarv.org>
 *
 * Use of this source code is restricted per the MIT license, a copy of which 
 * can be found at https://opensource.org/licenses/MIT (or should be included 
 * as LICENSE.txt within the associated archive or repository).
 */

#include "kernel_imp.h"

#include "kernel_imp.h_harness"
#include "kernel_imp.c_harness"

// ============================================================================

#define Nb (  4 )
#define Nk (  4 )
#define Nr ( 10 )

#define AES_ENC_RND_KEY_STEP(a,b,c,d        ) {   \
  s[ a ] = s[ a ] ^ rk[ a ];                      \
  s[ b ] = s[ b ] ^ rk[ b ];                      \
  s[ c ] = s[ c ] ^ rk[ c ];                      \
  s[ d ] = s[ d ] ^ rk[ d ];                      \
}
#define AES_ENC_RND_SUB_STEP(a,b,c,d        ) {   \
  s[ a ] = AES_ENC_SBOX[ s[ a ] ];                \
  s[ b ] = AES_ENC_SBOX[ s[ b ] ];                \
  s[ c ] = AES_ENC_SBOX[ s[ c ] ];                \
  s[ d ] = AES_ENC_SBOX[ s[ d ] ];                \
}
#define AES_ENC_RND_ROW_STEP(a,b,c,d,e,f,g,h) {   \
  uint8_t __a1 = s[ a ];                          \
  uint8_t __b1 = s[ b ];                          \
  uint8_t __c1 = s[ c ];                          \
  uint8_t __d1 = s[ d ];                          \
                                                  \
  s[ e ] = __a1;                                  \
  s[ f ] = __b1;                                  \
  s[ g ] = __c1;                                  \
  s[ h ] = __d1;                                  \
}
#define AES_ENC_RND_MIX_STEP(a,b,c,d        ) {   \
  uint8_t __a1 = s[ a ], __a2 = AES_MULX[ __a1 ]; \
  uint8_t __b1 = s[ b ], __b2 = AES_MULX[ __b1 ]; \
  uint8_t __c1 = s[ c ], __c2 = AES_MULX[ __c1 ]; \
  uint8_t __d1 = s[ d ], __d2 = AES_MULX[ __d1 ]; \
                                                  \
  uint8_t __a3 = __a1 ^ __a2;                     \
  uint8_t __b3 = __b1 ^ __b2;                     \
  uint8_t __c3 = __c1 ^ __c2;                     \
  uint8_t __d3 = __d1 ^ __d2;                     \
                                                  \
  s[ a ] = __a2 ^ __b3 ^ __c1 ^ __d1;             \
  s[ b ] = __a1 ^ __b2 ^ __c3 ^ __d1;             \
  s[ c ] = __a1 ^ __b1 ^ __c2 ^ __d3;             \
  s[ d ] = __a3 ^ __b1 ^ __c1 ^ __d2;             \
}

uint8_t AES_MULX[] = { 
  0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 
  0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,   
  0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E, 
  0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E,   
  0x40, 0x42, 0x44, 0x46, 0x48, 0x4A, 0x4C, 0x4E, 
  0x50, 0x52, 0x54, 0x56, 0x58, 0x5A, 0x5C, 0x5E,   
  0x60, 0x62, 0x64, 0x66, 0x68, 0x6A, 0x6C, 0x6E, 
  0x70, 0x72, 0x74, 0x76, 0x78, 0x7A, 0x7C, 0x7E,   
  0x80, 0x82, 0x84, 0x86, 0x88, 0x8A, 0x8C, 0x8E, 
  0x90, 0x92, 0x94, 0x96, 0x98, 0x9A, 0x9C, 0x9E,   
  0xA0, 0xA2, 0xA4, 0xA6, 0xA8, 0xAA, 0xAC, 0xAE, 
  0xB0, 0xB2, 0xB4, 0xB6, 0xB8, 0xBA, 0xBC, 0xBE,   
  0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE, 
  0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDC, 0xDE,   
  0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEC, 0xEE, 
  0xF0, 0xF2, 0xF4, 0xF6, 0xF8, 0xFA, 0xFC, 0xFE,   
  0x1B, 0x19, 0x1F, 0x1D, 0x13, 0x11, 0x17, 0x15, 
  0x0B, 0x09, 0x0F, 0x0D, 0x03, 0x01, 0x07, 0x05,   
  0x3B, 0x39, 0x3F, 0x3D, 0x33, 0x31, 0x37, 0x35, 
  0x2B, 0x29, 0x2F, 0x2D, 0x23, 0x21, 0x27, 0x25,   
  0x5B, 0x59, 0x5F, 0x5D, 0x53, 0x51, 0x57, 0x55, 
  0x4B, 0x49, 0x4F, 0x4D, 0x43, 0x41, 0x47, 0x45,   
  0x7B, 0x79, 0x7F, 0x7D, 0x73, 0x71, 0x77, 0x75, 
  0x6B, 0x69, 0x6F, 0x6D, 0x63, 0x61, 0x67, 0x65,   
  0x9B, 0x99, 0x9F, 0x9D, 0x93, 0x91, 0x97, 0x95, 
  0x8B, 0x89, 0x8F, 0x8D, 0x83, 0x81, 0x87, 0x85,   
  0xBB, 0xB9, 0xBF, 0xBD, 0xB3, 0xB1, 0xB7, 0xB5, 
  0xAB, 0xA9, 0xAF, 0xAD, 0xA3, 0xA1, 0xA7, 0xA5,   
  0xDB, 0xD9, 0xDF, 0xDD, 0xD3, 0xD1, 0xD7, 0xD5, 
  0xCB, 0xC9, 0xCF, 0xCD, 0xC3, 0xC1, 0xC7, 0xC5,   
  0xFB, 0xF9, 0xFF, 0xFD, 0xF3, 0xF1, 0xF7, 0xF5, 
  0xEB, 0xE9, 0xEF, 0xED, 0xE3, 0xE1, 0xE7, 0xE5 
}; 

uint8_t AES_ENC_SBOX[] = { 
  0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5,
  0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
  0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0,
  0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
  0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC,
  0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
  0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A,
  0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
  0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0,
  0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
  0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B,
  0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
  0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85,
  0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
  0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5,
  0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
  0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17,
  0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
  0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88,
  0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
  0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C,
  0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
  0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9,
  0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
  0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6,
  0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
  0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E,
  0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
  0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94,
  0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
  0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68,
  0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16 
};

void aes_enc_rnd_key( uint8_t* s, const uint8_t* rk ) {
  AES_ENC_RND_KEY_STEP(  0,  1,  2,  3 );
  AES_ENC_RND_KEY_STEP(  4,  5,  6,  7 );
  AES_ENC_RND_KEY_STEP(  8,  9, 10, 11 );
  AES_ENC_RND_KEY_STEP( 12, 13, 14, 15 );
}

void aes_enc_rnd_sub( uint8_t* s ) {
  AES_ENC_RND_SUB_STEP(  0,  1,  2,  3 );
  AES_ENC_RND_SUB_STEP(  4,  5,  6,  7 );
  AES_ENC_RND_SUB_STEP(  8,  9, 10, 11 );
  AES_ENC_RND_SUB_STEP( 12, 13, 14, 15 );
}

void aes_enc_rnd_row( uint8_t* s ) {
  AES_ENC_RND_ROW_STEP(  1,  5,  9, 13, 
                        13,  1,  5,  9 );
  AES_ENC_RND_ROW_STEP(  2,  6, 10, 14, 
                        10, 14,  2,  6 );
  AES_ENC_RND_ROW_STEP(  3,  7, 11, 15, 
                         7, 11, 15,  3 );
}

void aes_enc_rnd_mix( uint8_t* s ) {
  AES_ENC_RND_MIX_STEP(  0,  1,  2,  3 );
  AES_ENC_RND_MIX_STEP(  4,  5,  6,  7 );
  AES_ENC_RND_MIX_STEP(  8,  9, 10, 11 );
  AES_ENC_RND_MIX_STEP( 12, 13, 14, 15 );
}

uint8_t aes_enc_exp_step( uint8_t* r, const uint8_t* rk, uint8_t rc ) {
  r[  0 ] = rc ^ AES_ENC_SBOX[ rk[ 13 ] ] ^ rk[  0 ];
  r[  1 ] =      AES_ENC_SBOX[ rk[ 14 ] ] ^ rk[  1 ];
  r[  2 ] =      AES_ENC_SBOX[ rk[ 15 ] ] ^ rk[  2 ];
  r[  3 ] =      AES_ENC_SBOX[ rk[ 12 ] ] ^ rk[  3 ];

  rc = AES_MULX[ rc ];

  r[  4 ] =                     r[  0 ]   ^ rk[  4 ]; 
  r[  5 ] =                     r[  1 ]   ^ rk[  5 ];
  r[  6 ] =                     r[  2 ]   ^ rk[  6 ];
  r[  7 ] =                     r[  3 ]   ^ rk[  7 ];

  r[  8 ] =                     r[  4 ]   ^ rk[  8 ]; 
  r[  9 ] =                     r[  5 ]   ^ rk[  9 ];
  r[ 10 ] =                     r[  6 ]   ^ rk[ 10 ];
  r[ 11 ] =                     r[  7 ]   ^ rk[ 11 ];

  r[ 12 ] =                     r[  8 ]   ^ rk[ 12 ]; 
  r[ 13 ] =                     r[  9 ]   ^ rk[ 13 ];
  r[ 14 ] =                     r[ 10 ]   ^ rk[ 14 ];
  r[ 15 ] =                     r[ 11 ]   ^ rk[ 15 ];

  return rc;
}

/** @brief      Execute the kernel prologue, i.e.,
  *             any  pre-execution steps such as   allocation of memory.
  *
  * @return     a Boolean flag indicating success (\c true) or failure (\c false)
  *
  * @note       Execution of this function is excluded from the trigger period.
  */

bool kernel_prologue() {
  return true;
}

/** @brief      Execute the kernel itself.
  *
  * @return     a Boolean flag indicating success (\c true) or failure (\c false)
  *
  * @note       Execution of this function is excluded from the trigger period.
  */

bool kernel() {
  uint8_t s[ 4 * Nb ], rk[ 4 * Nb ], rc = 0x01;

  memcpy( rk, k, 16 * sizeof( uint8_t ) );
  memcpy(  s, m, 16 * sizeof( uint8_t ) );

  //      1 initial round
    aes_enc_rnd_key(  s, rk );
  // Nr - 1 middle  rounds
  for( int i = 1; i < Nr; i++ ) {
    aes_enc_rnd_sub(  s     );
    aes_enc_rnd_row(  s     );
    aes_enc_rnd_mix(  s     );
    rc = aes_enc_exp_step( rk, rk, rc );
    aes_enc_rnd_key(  s, rk );
  }
  //      1 final   round
    aes_enc_rnd_sub(  s     );
    aes_enc_rnd_row(  s     );
    rc = aes_enc_exp_step( rk, rk, rc );
    aes_enc_rnd_key(  s, rk );

  memcpy(  c, s, 16 * sizeof( uint8_t ) );

  return true;
}

/** @brief      Execute the kernel epilogue, i.e.,
  *             any post-execution steps such as deallocation of memory.
  *
  * @return     a Boolean flag indicating success (\c true) or failure (\c false)
  *
  * @note       Execution of this function is excluded from the trigger period.
  */

bool kernel_epilogue() {
  return true;
}

// ============================================================================
