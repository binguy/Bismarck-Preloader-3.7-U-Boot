#include <secure_boot.h>
#include <rsa-mod-exp.h>

/* extracted from include/compiler.h */
typedef uint8_t __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef unsigned int uint;

typedef int word_type __attribute__ ((mode (__word__)));

struct DWstruct {
	int high,low;
};

typedef union {
	struct DWstruct s;
	long long ll;
} DWunion;

# define cpu_to_le16(x)		uswap_16(x)
# define cpu_to_le32(x)		uswap_32(x)
# define cpu_to_le64(x)		uswap_64(x)
# define le16_to_cpu(x)		uswap_16(x)
# define le32_to_cpu(x)		uswap_32(x)
# define le64_to_cpu(x)		uswap_64(x)
# define cpu_to_be16(x)		(x)
# define cpu_to_be32(x)		(x)
# define cpu_to_be64(x)		(x)
# define be16_to_cpu(x)		(x)
# define be32_to_cpu(x)		(x)
# define be64_to_cpu(x)		(x)

#define fdt32_to_cpu(x) be32_to_cpu(x)
#define fdt64_to_cpu(x) be64_to_cpu(x)
#define cpu_to_fdt32(x) cpu_to_be32(x)

#define get_unaligned_be32(a) fdt32_to_cpu(*(uint32_t *)a)
#define put_unaligned_be32(a, b) (*(uint32_t *)(b) = cpu_to_fdt32(a))

struct rsa_public_key {
	uint len;		/* len of modulus[] in number of uint32_t */
	uint32_t n0inv;		/* -1 / modulus[0] mod 2^32 */
	uint32_t *modulus;	/* modulus as little endian array */
	uint32_t *rr;		/* R^2 as little endian array */
	uint64_t exponent;	/* public exponent */
};

#define _BYPASS_RSA_INIT_PROP_ 1
#if (_BYPASS_RSA_INIT_PROP_ != 1)
static struct key_prop *rsa_init_prop(const image_t *info, struct key_prop *prop) {
	return prop;
}
#endif

long long __ashldi3(long long u, word_type b) {
	DWunion uu, w;
	word_type bm;

	if (b == 0)
		return u;

	uu.ll = u;
	bm = 32 - b;

	if (bm <= 0) {
		w.s.low = 0;
		w.s.high = (unsigned int) uu.s.low << -bm;
	} else {
		const unsigned int carries = (unsigned int) uu.s.low >> bm;

		w.s.low = (unsigned int) uu.s.low << b;
		w.s.high = ((unsigned int) uu.s.high << b) | carries;
	}

	return w.ll;
}

static void _noinline_ subtract_modulus(const struct rsa_public_key *key, uint32_t num[]) {
	int64_t acc = 0;
	uint i;

	for (i = 0; i < key->len; i++) {
		acc += (uint64_t)num[i] - key->modulus[i];
		num[i] = (uint32_t)acc;
		acc >>= 32;
	}
}

static void _noinline_ rsa_convert_big_endian(uint32_t *dst, const uint32_t *src, int len) {
	int i;

	for (i = 0; i < len; i++)
		dst[i] = fdt32_to_cpu(src[len - 1 - i]);

	return;
}

static int _noinline_ num_public_exponent_bits(const struct rsa_public_key *key,	int *num_bits) {
	uint64_t exponent;
	int exponent_bits;
	const uint max_bits = (sizeof(exponent) * 8);

	exponent = key->exponent;
	exponent_bits = 0;

	if (!exponent) {
		*num_bits = exponent_bits;
		return 0;
	}

	for (exponent_bits = 1; exponent_bits < max_bits + 1; ++exponent_bits)
		if (!(exponent >>= 1)) {
			*num_bits = exponent_bits;
			return 0;
		}

	return 1;
}

static int _noinline_ is_public_exponent_bit_set(const struct rsa_public_key *key, int pos) {
	return key->exponent & (1ULL << pos);
}

static void _noinline_ montgomery_mul_add_step(const struct rsa_public_key *key, uint32_t result[],
																		const uint32_t a, const uint32_t b[]) {
	uint64_t acc_a, acc_b;
	uint32_t d0;
	uint i;

	acc_a = (uint64_t)a * b[0] + result[0];
	d0 = (uint32_t)acc_a * key->n0inv;
	acc_b = (uint64_t)d0 * key->modulus[0] + (uint32_t)acc_a;
	for (i = 1; i < key->len; i++) {
		acc_a = (acc_a >> 32) + (uint64_t)a * b[i] + result[i];
		acc_b = (acc_b >> 32) + (uint64_t)d0 * key->modulus[i] +
			(uint32_t)acc_a;
		result[i - 1] = (uint32_t)acc_b;
	}

	acc_a = (acc_a >> 32) + (acc_b >> 32);

	result[i - 1] = (uint32_t)acc_a;

	if (acc_a >> 32)
		subtract_modulus(key, result);
}

static void _noinline_ montgomery_mul(const struct rsa_public_key *key,
													 uint32_t result[], uint32_t a[], const uint32_t b[]) {
	uint i;

	for (i = 0; i < key->len; ++i)
		result[i] = 0;
	for (i = 0; i < key->len; ++i)
		montgomery_mul_add_step(key, result, a[i], b);
}

static int _noinline_ greater_equal_modulus(const struct rsa_public_key *key,
																 uint32_t num[]) {
	int i;

	for (i = (int)key->len - 1; i >= 0; i--) {
		if (num[i] < key->modulus[i])
			return 0;
		if (num[i] > key->modulus[i])
			return 1;
	}

	return 1;  /* equal */
}

int _noinline_ pow_mod(const struct rsa_public_key *key, uint32_t *inout) {
	uint32_t *result, *ptr;
	uint i;
	int j, k;
	uint32_t val[key->len], acc[key->len], tmp[key->len];
	uint32_t a_scaled[key->len];

	result = tmp;  /* Re-use location. */

	/* Convert from big endian byte array to little endian word array. */
	for (i = 0, ptr = inout + key->len - 1; i < key->len; i++, ptr--)
		val[i] = get_unaligned_be32(ptr);

	if (0 != num_public_exponent_bits(key, &k))
		return 1;

	if (k < 2) {
		printf("EE: public exponent is too short (%d bits, minimum 2)\n", k);
		return 1;
	}

	if (!is_public_exponent_bit_set(key, 0)) {
		printf("EE: LSB of RSA public exponent must be set.\n");
		return 1;
	}

	/* the bit at e[k-1] is 1 by definition, so start with: C := M */
	montgomery_mul(key, acc, val, key->rr); /* acc = a * RR / R mod n */
	/* retain scaled version for intermediate use */
	memcpy((char *)a_scaled, (const char *)acc, key->len * sizeof(a_scaled[0]));

	for (j = k - 2; j > 0; --j) {
		montgomery_mul(key, tmp, acc, acc); /* tmp = acc^2 / R mod n */

		if (is_public_exponent_bit_set(key, j)) {
			/* acc = tmp * val / R mod n */
			montgomery_mul(key, acc, tmp, a_scaled);
		} else {
			/* e[j] == 0, copy tmp back to acc for next operation */
			memcpy((char *)acc, (const char *)tmp, key->len * sizeof(acc[0]));
		}
	}

	/* the bit at e[0] is always 1 */
	montgomery_mul(key, tmp, acc, acc); /* tmp = acc^2 / R mod n */
	montgomery_mul(key, acc, tmp, val); /* acc = tmp * a / R mod M */
	memcpy((char *)result, (const char *)acc, key->len * sizeof(result[0]));

	/* Make sure result < mod; result is at most 1x mod too large. */
	if (greater_equal_modulus(key, result))
		subtract_modulus(key, result);

	/* Convert to bigendian byte array */
	for (i = key->len - 1, ptr = inout; (int)i >= 0; i--, ptr++)
		put_unaligned_be32(result[i], ptr);
	return 0;
}

static int rsa_mod_exp_sw(const uint8_t *sig, uint32_t sig_len, struct key_prop *prop, uint8_t *out) {
	struct rsa_public_key key;
	uint32_t *key1, *key2, *buf;
	int ret;

	key.n0inv = prop->n0inv;
	key.len = prop->num_bits;
	key.exponent = fdt64_to_cpu(*((uint64_t *)(prop->public_exponent)));
	key.len /= sizeof(uint32_t) * 8;

	key1 = alloca(sizeof(uint32_t) * key.len);
	key2 = alloca(sizeof(uint32_t) * key.len);

	key.modulus = key1;
	key.rr = key2;
	rsa_convert_big_endian(key.modulus, (uint32_t *)prop->modulus, key.len);
	rsa_convert_big_endian(key.rr, (uint32_t *)prop->rr, key.len);
	if (!key.modulus || !key.rr) {
		return 1;
	}

	buf = (uint32_t *)alloca(sig_len);

	memcpy((char *)buf, (const char *)sig, sig_len);

	ret = pow_mod(&key, buf);
	if (ret)
		return ret;

	memcpy((char *)out, (const char *)buf, sig_len);

	return 0;
}

static _noinline_ int rsa_verify_padding(const uint8_t *msg, const int pad_len,
                              const struct checksum_algo *algo)
{
	int ff_len;
	int ret;

	/* first byte must be 0x00 */
	ret = *msg++;
	/* second byte must be 0x01 */
	ret |= *msg++ ^ 0x01;
	/* next ff_len bytes must be 0xff */
	ff_len = pad_len - algo->der_len - 3;
	ret |= *msg ^ 0xff;
	ret |= memcmp(msg, msg+1, ff_len-1);
	msg += ff_len;
	/* next byte must be 0x00 */
	ret |= *msg++;
	/* next der_len bytes must match der_prefix */
	ret |= memcmp(msg, algo->der_prefix, algo->der_len);

	return ret;
}

int _noinline_ rsa_verify_key(struct key_prop *prop, const image_t *info,
                          const int key_len, const struct checksum_algo *hash_algo,
                          const uint8_t *hash_val) {
	uint8_t *buf;
	int ret, pad_len;

	buf = alloca(info->sig_len);

	ret = rsa_mod_exp_sw(&info->sig_signature[0], info->sig_len, prop, buf);
	if (ret) {
		printf("EE: Modular exponentation failed\n");
		return ret;
	}

	pad_len = key_len - hash_algo->checksum_len;

	ret = rsa_verify_padding(buf, pad_len, hash_algo);
	if (ret) {
		printf("EE: Paddign check failed\n");
		return ret;
	}

	if (memcmp((uint8_t *)buf + pad_len, hash_val, info->sig_len - pad_len)) {
		printf("EE: Hash check failed\n");
		return 1;
	}

	return 0;
}


int rsa_verify(const image_t *info,
							 const int key_len,
               const struct checksum_algo *hash_algo,
               const uint8_t *hash_val) {
	int ret = 0;
	struct key_prop prop _unused_;
	const uint32_t pub_exp[] = {0x00000000, 0x00010001};
	const uint32_t modulus[] = {0xab2bcc86, 0x887f23e7, 0xef1e038a, 0xde9cac04, 0xae0743d1,
															0x026779aa, 0xf16ca6d2, 0xefe38086, 0x4594a42f, 0x2efafbb3,
															0x1a7ab671, 0x9077bd62, 0x779e08da, 0xb0d22bdb, 0x90f3d581,
															0xb778498f, 0xc96d5d5f, 0x0cfe0aad, 0x750d4f1c, 0x578e5ece,
															0x84b177e7, 0x0730d873, 0xf9d2a4ef, 0xe381d445, 0xc3229ef9,
															0x87a40dbb, 0x6878cc23, 0xfccfa9ca, 0x7a84ae18, 0xe2b1235e,
															0xda6495ab, 0x12efc224, 0x3301fa7b, 0x4da55518, 0x414c2f79,
															0xea8df02f, 0x559be27b, 0x3aa7f6e9, 0x29000473, 0x029e85f8,
															0x4d140c66, 0x7f892b8f, 0x60a05458, 0x51fbfd41, 0x47474c4b,
															0xd314d416, 0xd906ef7b, 0xa0e9daee, 0x1bc5ed33, 0x42134ecb,
															0x1da7af95, 0xea8b3f19, 0x0d9a523e, 0x3ccb85d9, 0x799df0eb,
															0x4d0f1b21, 0xbacc565b, 0x4f395f99, 0x43ae75df, 0x2a309110,
															0xa2c89b73, 0xea377071, 0xfa5d9ae6, 0x053dbf67};
	const uint32_t rr[] = {0x0681b44a, 0xf689fa88, 0xf9fdcada, 0x03f5b908, 0xc5c05b84,
                         0x6b2be556, 0xe342e452, 0x26bad86a, 0xff650995, 0x24834ad3,
												 0xb7372762, 0xf45a3bdb, 0x7535b6b2, 0xb488caa1, 0xb0f7f77f,
												 0x68dec422, 0xda1e9aa9, 0xb7993b30, 0x69291095, 0xbab066e7,
												 0x234acf51, 0xa06efdbe, 0xdbb8e10b, 0xff1099b3, 0x1017b3d9,
												 0x0e05f7d3, 0x17706753, 0x0953a3a0, 0xbf0ccd40, 0xbffe7205,
												 0x473739a1, 0x6e2568a1, 0xa562afe9, 0x79339c62, 0x86b4cf4b,
												 0xa371d4d2, 0xe036de52, 0xdb287955, 0xc9ebfaa8, 0x13fcb956,
												 0x5f6bde33, 0x9be53cb3, 0xd4f190c5, 0x09bbf3bd, 0x038276ec,
												 0x690cd4bc, 0xc0234a3a, 0x4d8199ae, 0x66652225, 0x3723094b,
												 0x423adb57, 0x0b7ff10d, 0x9f66eee7, 0x94304968, 0xed78020d,
												 0xec64a086, 0x5b02b0a6, 0xd492f4e3, 0x592b97fc, 0xbd5a578d,
												 0xa3049e84, 0x265bc660, 0xa84437e0, 0xec0cda9c};

#if (_BYPASS_RSA_INIT_PROP_ == 1)
	/* prop.num_bits = fdtdec_get_int(blob, node, "rsa,num-bits", 0); */
	prop.num_bits = 0x800;

	/* prop.n0inv = fdtdec_get_int(blob, node, "rsa,n0-inverse", 0); */
	prop.n0inv = 0xa5b813a9;

	/* prop.public_exponent = fdt_getprop(blob, node, "rsa,exponent", &length); */
	/* if (!prop.public_exponent || length < sizeof(uint64_t)) */
	/* 	prop.public_exponent = NULL; */
	prop.public_exponent = pub_exp;

	prop.exp_len = sizeof(uint64_t);

	/* prop.modulus = fdt_getprop(blob, node, "rsa,modulus", NULL); */
	prop.modulus = modulus;

	/* prop.rr = fdt_getprop(blob, node, "rsa,r-squared", NULL); */
	prop.rr = rr;
#else
	rsa_init_prop(info, prop);
#endif

	ret = rsa_verify_key(&prop, info, key_len, hash_algo, hash_val);

	return ret;
}
