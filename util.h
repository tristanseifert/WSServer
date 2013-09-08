//
//  util.h
//  APNSServer
//
//  Created by Tristan Seifert on 12/07/2013.
//  Copyright (c) 2013 Squee! Apps. All rights reserved.
//

#ifndef APNSServer_util_h
#define APNSServer_util_h

void util_string_trim(char* string);
char *util_config_strip_quotes(char *string);
char* util_base64_encode(const unsigned char* input, int length);
char* util_sha1(char* data, int length);

#endif
