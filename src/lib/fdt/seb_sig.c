#include <secure_boot.h>

#define SHA1_SUM_LEN	20
#define SHA1_DER_LEN	15

#define RSA2048_BYTES (2048/8)
#define RSA4096_BYTES (4096/8)

/* It seems for RSA padding only. Not going to put it on SHA1 side. */
static const uint8_t sha1_der_prefix[SHA1_DER_LEN] = {
	0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e,
	0x03, 0x02, 0x1a, 0x05, 0x00, 0x04, 0x14
};

const struct checksum_algo checksum_algos[] = {
	{
		.name = "sha1",
		.checksum_len = SHA1_SUM_LEN,
		.der_len = SHA1_DER_LEN,
		.der_prefix = sha1_der_prefix,
		.calculate = sha1_calculate,
	/* }, { */
	/* 	.name = "sha256", */
	/* 	.checksum_len = SHA256_SUM_LEN, */
	/* 	.der_len = SHA256_DER_LEN, */
	/* 	.der_prefix = sha256_der_prefix, */
	/* 	.calculate = hash_calculate, */
	}
};

const struct crypto_algo crypto_algos[] = {
	{
		.name = "rsa2048",
		.key_len = RSA2048_BYTES,
		/* .sign = rsa_sign, */
		/* .add_verify_data = rsa_add_verify_data, */
		.verify = rsa_verify,
/* 	}, { */
/* 		.name = "rsa4096", */
/* 		.key_len = RSA4096_BYTES, */
/* 		.sign = rsa_sign, */
/* 		.add_verify_data = rsa_add_verify_data, */
/* 		.verify = rsa_verify, */
	}
};

static const struct checksum_algo *seb_sig_get_hash_algo(image_t *info) {
	/* shall change to traverse LDS */
	return &checksum_algos[info->sig_hash_algo - 1];
}

static const struct crypto_algo *seb_sig_get_crypto_algo(image_t *info) {
	/* shall change to traverse LDS */
	return &crypto_algos[info->sig_crypto_algo - 1];
}

int seb_verify_image(image_t *info) {
	int ret;
	uint8_t *hash_value;
	const struct checksum_algo *hash_algo;
	const struct crypto_algo *crypto_algo;

	hash_algo = seb_sig_get_hash_algo(info);
	hash_value = (uint8_t *)alloca(hash_algo->checksum_len);

	crypto_algo = seb_sig_get_crypto_algo(info);

	ret = hash_algo->calculate(info, hash_value);
	if (ret)
		return ret;

	ret = crypto_algo->verify(info, crypto_algo->key_len, hash_algo, hash_value);
	if (ret)
		return ret;

	return 0;
}
