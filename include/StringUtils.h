/*
 *	StringUtils.h
 *		string handling helpers
 *
 *	Copyright (c) 2024-2026, DexRobot.Inc
 *
 */
#pragma once

#include <vector>
#include <string>

#include "commondef.h"

//
#define BITSPERBYTE 8
#define HASH_SHIFT  5


namespace DexRobot
{

#ifdef WIN32
extern bool str_endswith(const char *str, const char *end);
extern int  strtoint(const char *__restrict str, char **__restrict endptr, int base);
extern void clean_ascii(char *str);
extern size_t strip_crlf(char *str);

extern bool is_empty_str(const char *);
extern bool is_empty_str(const std::string &);

bool is_numeric(const char * str);

void split(std::vector<std::string>& res, const std::string & str, const std::string & delimiter=" ");
void split(std::vector<std::string>& res, const std::string & str, char delimiter=' ');

void str_replace(std::string & str, const char src, const char dest);

// function collpase removes all spaces in a given string, resulting an intensive string without space,
// for the purpose of creating hash for a sentence in some scenario.
std::string & collapse(std::string &);
std::string & collapse_v2(std::string &);
char* collapse(const char *src, char *dest);

void mash(UInt32& hash, UInt32 chars);

UInt32 hashFoldCase(std::string);
UInt32 hash(std::string &);
UInt32 hash(std::string &, bool);

void padding_head(std::string & str, size_t width);
std::string padding_head(const std::string & str, size_t width);

void padding_tail(std::string & str, size_t width);
std::string padding_tail(const std::string & str, size_t width);

std::string filling_head(const std::string & str, char ch, size_t width);
std::string filling_tail(const std::string & str, char ch, size_t width);

#else
extern bool str_endswith(const char *str, const char *end);
extern int	strtoint(const char *__restrict str, char **__restrict endptr, int base);
extern void clean_ascii(char *str);
extern size_t strip_crlf(char *str);

extern bool is_empty_str(const char *);
extern bool is_empty_str(const std::string &);

bool is_numeric(const char * str);

void split(std::vector<std::string>& res, const std::string & str, const std::string & delimiter=" ");
void split(std::vector<std::string>& res, const std::string & str, char delimiter=' ');

void str_replace(std::string & str, const char src, const char dest);

// function collpase removes all spaces in a given string, resulting an intensive string without space,
// for the purpose of creating hash for a sentence in some scenario.
std::string & collapse(std::string &);
std::string & collapse_v2(std::string &);
char* collapse(const char *src, char *dest);

void mash(UInt32& hash, UInt32 chars);

UInt32 hashFoldCase(std::string);
UInt32 hash(std::string &);
UInt32 hash(std::string &, bool);

void padding_head(std::string & str, size_t width);
std::string padding_head(const std::string & str, size_t width);

void padding_tail(std::string & str, size_t width);
std::string padding_tail(const std::string & str, size_t width);

std::string filling_head(const std::string & str, char ch, size_t width);
std::string filling_tail(const std::string & str, char ch, size_t width);

#endif

}
