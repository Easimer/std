/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/SwissTable.hpp"
#include "std/serializer/Serializer2.hpp"

namespace sn {

template <typename K, typename V>
[[nodiscard]] inline Optional<Error> serialize(Serialization *S,
                                               SwissTable<K, V> *table) {
  if (S->isWriting) {
    size_t numEntries = table->size();
    size_t numWritten = 0;

    SN_TRY_SERIALIZE(serialize(S, &numEntries));

    for (auto [k, v] : SwissTableIterable(*table)) {
      SN_TRY_SERIALIZE(serialize(S, &k));
      SN_TRY_SERIALIZE(serialize(S, &v));
      numWritten++;
    }

    (void)numWritten;
    DCHECK(numWritten == numEntries);
  } else {
    size_t numEntries = 0;
    SN_TRY_SERIALIZE(serialize(S, &numEntries));

    for (size_t i = 0; i < numEntries; i++) {
      Slice<K> k;
      V v;
      SN_TRY_SERIALIZE(serialize(S, &k));
      SN_TRY_SERIALIZE(serialize(S, &v));

      table->put(k, v);
    }
  }

  return {};
}
}  // namespace sn
