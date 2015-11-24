/*
 * Copyright 2015 Wink Saville
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ac_string.h>

/**
 * Compare two strings, returns 0 if they are identical.
 * return < 0 if the first non-matching character of str1
 * is < that of str2 or > 0 if str1 > str2.
 */
ac_size_t ac_strncmp(const char *str1, const char *str2, ac_size_t num) {
    int diff = 0;
    if (num > 0) {
        for (int i = 0; i < num; i++) {
            char ch1 = str1[i];
            char ch2 = str2[i];
            diff = ch1 - ch2;
            if (diff != 0) {
                return diff;
            }
        }
    }
    return diff;
}
