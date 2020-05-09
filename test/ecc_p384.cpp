//---------------------------------------------------------------------------//
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2020 Nikita Kaskov <nbering@nil.foundation>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#include "fuzzers.hpp"
#include "ecc_helper.hpp"

void fuzz(const uint8_t in[], size_t len) {
    if (len > 2 * 384 / 8) {
        return;
    }
    static nil::crypto3::ec_group p384("secp384r1");
    return check_ecc_math(p384, in, len);
}
