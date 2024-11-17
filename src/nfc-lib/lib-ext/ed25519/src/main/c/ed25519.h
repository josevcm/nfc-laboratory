/*
    Copyright (c) 2015 Orson Peters <orsonpeters@gmail.com>

    This software is provided 'as-is', without any express or implied warranty. In no event will the
    authors be held liable for any damages arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose, including commercial
    applications, and to alter it and redistribute it freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not claim that you wrote the
    original software. If you use this software in a product, an acknowledgment in the product
    documentation would be appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be misrepresented as
    being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/
#ifndef ED25519_H
#define ED25519_H

#include <stddef.h>

#if defined(_WIN32)
    #if defined(ED25519_BUILD_DLL)
        #define ED25519_DECLSPEC __declspec(dllexport)
    #elif defined(ED25519_DLL)
        #define ED25519_DECLSPEC __declspec(dllimport)
    #else
        #define ED25519_DECLSPEC
    #endif
#else
    #define ED25519_DECLSPEC
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifndef ED25519_NO_SEED
int ED25519_DECLSPEC ed25519_create_seed(unsigned char *seed);
#endif

void ED25519_DECLSPEC ed25519_create_keypair(unsigned char *public_key, unsigned char *private_key, const unsigned char *seed);
void ED25519_DECLSPEC ed25519_sign(unsigned char *signature, const unsigned char *message, size_t message_len, const unsigned char *public_key, const unsigned char *private_key);
int ED25519_DECLSPEC ed25519_verify(const unsigned char *signature, const unsigned char *message, size_t message_len, const unsigned char *public_key);
void ED25519_DECLSPEC ed25519_add_scalar(unsigned char *public_key, unsigned char *private_key, const unsigned char *scalar);
void ED25519_DECLSPEC ed25519_key_exchange(unsigned char *shared_secret, const unsigned char *public_key, const unsigned char *private_key);


#ifdef __cplusplus
}
#endif

#endif
