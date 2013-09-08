//
//  util.c
//  APNSServer
//
//  Created by Tristan Seifert on 12/07/2013.
//  Copyright (c) 2013 Squee! Apps. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>
#include <pthread.h>

void util_string_trim(char* string) {
	// remove leading spaces
	const char* firstNonSpace = string;
	while(*firstNonSpace != '\0' && isspace(*firstNonSpace)) {
		++firstNonSpace;
	}
	
	size_t len = strlen(firstNonSpace)+1;
	memmove(string, firstNonSpace, len);
	
	// trailing spaces
	
	char* endOfString = string + len;
	while(string < endOfString  && isspace(*endOfString)) {
		--endOfString;
	}
	
	*endOfString = '\0';
}

char *util_config_strip_quotes(char *string) {
	int length = (int) strlen(string);
	
	char *newString = malloc(length);
	strcpy(newString, string);
	
	char *strPtr = newString;
	strPtr += length - 1;
	*strPtr = 0x00;
	return newString + 1;
}

char* util_base64_encode(const unsigned char* input, int length) {
	BIO *bmem, *b64;
	BUF_MEM *bptr;

	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, input, length);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);

	char *buff = (char *) malloc(bptr->length);
	memcpy(buff, bptr->data, bptr->length-1);
	buff[bptr->length-1] = 0;

	BIO_free_all(b64);

	return buff;
}

char* util_sha1(char* data, int length) {
	char* outBuf = malloc(SHA_DIGEST_LENGTH + 2);
	memset(outBuf, 0x00, SHA_DIGEST_LENGTH + 2);

	SHA1(data, length, outBuf);

	return outBuf;
}