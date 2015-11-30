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

#ifndef SADIE_LIBS_INCS_AC_H
#define SADIE_LIBS_INCS_AC_H

#include <ac_inttypes.h>
#include <ac_msg.h>

/**
 * Forward declaration of an asynchronous component
 */
typedef struct _ac ac;

/**
 * Process a message.
 *
 * return AC_FALSE if the message was NOT fully handled in which
 * case the parent component will be called otherwise processing
 * is complete.
 */
typedef ac_bool (*ac_process_msg_fn)(ac* this, ac_msg* pmsg);

/**
 * Get a components data
 *
 * returns a void pointer to the data associated with this compoent.
 */
typedef void* (*ac_get_data_fn)(ac* this);

/**
 * An Asynchronouse Component
 */
typedef struct _ac {
  ac_process_msg_fn process_msg;  // Process a message
  ac_get_data_fn get_data;        // Return the components data
} ac;

#define ac_data(pac, type) ((type)(pac->get_data(pac)))

#endif
