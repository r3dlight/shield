// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: LicenseRef-LEDGER

#include <gtest/gtest.h>
#include <random>
#include <iostream>
#include <shield/string.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

static const struct {
    const char *string;
    int         len;
} samples_len[] = {
    { "foobar", 6 },
    { NULL, 0 },
    { "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 62 }
};

TEST(TestString, Strlen) {
    for (size_t n = 0; n < ARRAY_SIZE(samples_len); ++n) {
        ASSERT_EQ(shield_strlen(samples_len[n].string), samples_len[n].len);
    }
}


static const struct {
    const char *str1;
    const char *str2;
    int         cmp;
} samples_cmp[] = {
    {NULL, "hello", -1},
    {"hello", NULL, -1},
    {NULL, NULL, -1},
    {"hello", "hello", 0},
    {"hello", "world", -1},
    {"world", "hello", 1},
    {"openai", "openai", 0},
    {"chatbot", "chatbots", -1},
    {"programming", "programming", 0},
    {"apple", "banana", -1},
    {"zebra", "lion", 1}
};

TEST(TestString, Strcmp) {
    for (size_t n = 0; n < ARRAY_SIZE(samples_cmp); ++n) {
        ASSERT_EQ(shield_strcmp(samples_cmp[n].str1, samples_cmp[n].str2), samples_cmp[n].cmp);
    }
}

static char target[5];

static const struct {
    char *str1;
    const char *str2;
    char *ret;
} samples_cpy[] = {
    {NULL, "hello", NULL},
    {target, NULL, target},
    {NULL, NULL, NULL},
    {target, "hello", target},
    {target, "chatbots", target},
    {target, "programming", target},
};

TEST(TestString, Strcpy) {
    for (size_t n = 0; n < ARRAY_SIZE(samples_cpy); ++n) {
        ASSERT_EQ((size_t)shield_strcpy(samples_cpy[n].str1, samples_cpy[n].str2), (size_t)samples_cpy[n].ret);
    }
}
