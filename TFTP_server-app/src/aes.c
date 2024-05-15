/*
 * aes.c
 *
 *  Created on: 12 May 2024
 *      Author: Efe Tunca
 */

#include "aes.h"
#include "string.h"

static void KeyExpansion(uint8_t *RoundKey, const uint8_t *Key)
{
	unsigned i, j, k;
	uint8_t tempa[4]; // Used for the column/row operations

	// The first round key is the key itself.
	for (i = 0; i < Nk; ++i) {
		RoundKey[(i * 4) + 0] = Key[(i * 4) + 0];
		RoundKey[(i * 4) + 1] = Key[(i * 4) + 1];
		RoundKey[(i * 4) + 2] = Key[(i * 4) + 2];
		RoundKey[(i * 4) + 3] = Key[(i * 4) + 3];
	}

	// All other round keys are found from the previous round keys.
	for (i = Nk; i < Nb * (Nr + 1); ++i) {
		{
			k = (i - 1) * 4;
			tempa[0] = RoundKey[k + 0];
			tempa[1] = RoundKey[k + 1];
			tempa[2] = RoundKey[k + 2];
			tempa[3] = RoundKey[k + 3];
		}

		if (i % Nk == 0) {
			// This function shifts the 4 bytes in a word to the left once.
			// [a0,a1,a2,a3] becomes [a1,a2,a3,a0]

			// Function RotWord()
			{
				const uint8_t u8tmp = tempa[0];
				tempa[0] = tempa[1];
				tempa[1] = tempa[2];
				tempa[2] = tempa[3];
				tempa[3] = u8tmp;
			}

			// SubWord() is a function that takes a four-byte input word and
			// applies the S-box to each of the four bytes to produce an output word.

			// Function Subword()
			{
				tempa[0] = getSBoxValue(tempa[0]);
				tempa[1] = getSBoxValue(tempa[1]);
				tempa[2] = getSBoxValue(tempa[2]);
				tempa[3] = getSBoxValue(tempa[3]);
			}

			tempa[0] = tempa[0] ^ Rcon[i/Nk];
		}
		if (i % Nk == 4) {
			// Function Subword()
			{
				tempa[0] = getSBoxValue(tempa[0]);
				tempa[1] = getSBoxValue(tempa[1]);
				tempa[2] = getSBoxValue(tempa[2]);
				tempa[3] = getSBoxValue(tempa[3]);
			}
		}

		j = i * 4; k=(i - Nk) * 4;
		RoundKey[j + 0] = RoundKey[k + 0] ^ tempa[0];
		RoundKey[j + 1] = RoundKey[k + 1] ^ tempa[1];
		RoundKey[j + 2] = RoundKey[k + 2] ^ tempa[2];
		RoundKey[j + 3] = RoundKey[k + 3] ^ tempa[3];
	}
}

// MixColumns function mixes the columns of the state matrix.
// The method used to multiply may be difficult to understand for the inexperienced.
static void InvMixColumns(state_t *state)
{
	int i;
	uint8_t a, b, c, d;

	for (i = 0; i < 4; ++i) {
		a = (*state)[i][0];
		b = (*state)[i][1];
		c = (*state)[i][2];
		d = (*state)[i][3];

		(*state)[i][0] = Multiply(a, 0x0e) ^ Multiply(b, 0x0b) ^ Multiply(c, 0x0d) ^ Multiply(d, 0x09);
		(*state)[i][1] = Multiply(a, 0x09) ^ Multiply(b, 0x0e) ^ Multiply(c, 0x0b) ^ Multiply(d, 0x0d);
		(*state)[i][2] = Multiply(a, 0x0d) ^ Multiply(b, 0x09) ^ Multiply(c, 0x0e) ^ Multiply(d, 0x0b);
		(*state)[i][3] = Multiply(a, 0x0b) ^ Multiply(b, 0x0d) ^ Multiply(c, 0x09) ^ Multiply(d, 0x0e);
	}
}

// This function adds the round key to state.
// The round key is added to the state by an XOR function.
static void AddRoundKey(uint8_t round, state_t *state, const uint8_t *RoundKey)
{
	uint8_t i,j;

	for (i = 0; i < 4; ++i) {
		for (j = 0; j < 4; ++j)
		  (*state)[i][j] ^= RoundKey[(round * Nb * 4) + (i * Nb) + j];
	}
}

// The SubBytes Function Substitutes the values in the
// state matrix with values in an S-box.
static void InvSubBytes(state_t *state)
{
	uint8_t i, j;

	for (i = 0; i < 4; ++i) {
		for (j = 0; j < 4; ++j)
		  (*state)[j][i] = getSBoxInvert((*state)[j][i]);
	}
}

static void InvShiftRows(state_t *state)
{
	uint8_t temp;

	// Rotate first row 1 columns to right
	temp = (*state)[3][1];
	(*state)[3][1] = (*state)[2][1];
	(*state)[2][1] = (*state)[1][1];
	(*state)[1][1] = (*state)[0][1];
	(*state)[0][1] = temp;

	// Rotate second row 2 columns to right
	temp = (*state)[0][2];
	(*state)[0][2] = (*state)[2][2];
	(*state)[2][2] = temp;

	temp = (*state)[1][2];
	(*state)[1][2] = (*state)[3][2];
	(*state)[3][2] = temp;

	// Rotate third row 3 columns to right
	temp = (*state)[0][3];
	(*state)[0][3] = (*state)[1][3];
	(*state)[1][3] = (*state)[2][3];
	(*state)[2][3] = (*state)[3][3];
	(*state)[3][3] = temp;
}

static void InvCipher(state_t *state, const uint8_t *RoundKey)
{
	uint8_t round = 0;

	// Add the First round key to the state before starting the rounds.
	AddRoundKey(Nr, state, RoundKey);

	// There will be Nr rounds.
	// The first Nr-1 rounds are identical.
	// These Nr rounds are executed in the loop below.
	// Last one without InvMixColumn()
	for (round = (Nr - 1); ; --round)
	{
		InvShiftRows(state);
		InvSubBytes(state);
		AddRoundKey(round, state, RoundKey);
		if (round == 0)
		  break;
		InvMixColumns(state);
	}
}

static void XorWithIv(uint8_t *buf, const uint8_t *Iv)
{
	uint8_t blockNumber;

	for (blockNumber = 0; blockNumber < AES_BLOCKLEN; ++blockNumber) // The block in AES is always 128bit no matter the key size
		buf[blockNumber] ^= Iv[blockNumber];
}

void aes_init_ctx_iv(struct AES_ctx *ctx, const uint8_t *key, const uint8_t *iv)
{
	KeyExpansion(ctx->RoundKey, key);
	memcpy(ctx->Iv, iv, AES_BLOCKLEN);
}

void decrypt_aes(struct AES_ctx *ctx, uint8_t *buf, size_t length)
{
	size_t blockNumber;
	uint8_t storeNextIv[AES_BLOCKLEN];

	for (blockNumber = 0; blockNumber < length; blockNumber += AES_BLOCKLEN)
	{
		memcpy(storeNextIv, buf, AES_BLOCKLEN);
		InvCipher((state_t*)buf, ctx->RoundKey);
		XorWithIv(buf, ctx->Iv);
		memcpy(ctx->Iv, storeNextIv, AES_BLOCKLEN);
		buf += AES_BLOCKLEN;
	}
}
