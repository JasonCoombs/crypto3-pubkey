//---------------------------------------------------------------------------//
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2020 Nikita Kaskov <nbering@nil.foundation>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_FUZZER_DRIVER_HPP
#define CRYPTO3_FUZZER_DRIVER_HPP

#include <stdint.h>
#include <stdlib.h> // for setenv
#include <iostream>
#include <vector>

#include <nil/crypto3/utilities/exceptions.hpp>
#include <nil/crypto3/random/chacha_rng/chacha_rng.hpp>

static const size_t max_fuzzer_input_size = 8192;

extern void fuzz(const uint8_t in[], size_t len);

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv);

extern "C" int LLVMFuzzerTestOneInput(const uint8_t in[], size_t len);

extern "C" int LLVMFuzzerInitialize(int *, char ***) {
    /*
    * This disables the mlock pool, as overwrites within the pool are
    * opaque to ASan or other instrumentation.
    */
    ::setenv("CRYPTO3_MLOCK_POOL_SIZE", "0", 1);
    return 0;
}

// Called by main() in libFuzzer or in main for AFL below
extern "C" int LLVMFuzzerTestOneInput(const uint8_t in[], size_t len) {
    if (len <= max_fuzzer_input_size) {
        fuzz(in, len);
    }
    return 0;
}

// Some helpers for the fuzzer jigs

inline nil::crypto3::RandomNumberGenerator &fuzzer_rng() {
    static nil::crypto3::ChaCha_RNG rng(nil::crypto3::secure_vector<uint8_t>(32));
    return rng;
}

#define FUZZER_WRITE_AND_CRASH(expr) \
   do { std::cerr << expr; abort(); } while(0)

#define FUZZER_ASSERT_EQUAL(x, y) do {                                  \
   if(x != y) {                                                         \
      FUZZER_WRITE_AND_CRASH(#x << " = " << x << " !=\n"                \
                             << #y << " = " << y << "\n");              \
   } } while(0)

#define FUZZER_ASSERT_TRUE(e)                                           \
   do {                                                                 \
   if(!(e)) {                                                           \
      FUZZER_WRITE_AND_CRASH("Expression " << #e << " was false");      \
   } } while(0)

#if defined(CRYPTO3_FUZZER_IS_AFL) || defined(CRYPTO3_FUZZER_IS_TEST)

/* Stub for AFL */

#if defined(CRYPTO3_FUZZER_IS_AFL) && !defined(__AFL_COMPILER)
#error "Build configured for AFL but not being compiled by AFL compiler"
#endif

int main(int argc, char* argv[])
   {
   LLVMFuzzerInitialize(&argc, &argv);

#if defined(__AFL_LOOP)
   while(__AFL_LOOP(1000))
#endif
      {
      std::vector<uint8_t> buf(max_fuzzer_input_size);
      std::cin.read((char*)buf.data(), buf.size());
      const size_t got = std::cin.gcount();

      buf.resize(got);
      buf.shrink_to_fit();

      LLVMFuzzerTestOneInput(buf.data(), got);
      }
   }

#elif defined(CRYPTO3_FUZZER_IS_KLEE)

#include <klee/klee.h>

int main(int argc, char* argv[])
   {
   LLVMFuzzerInitialize(&argc, &argv);

   uint8_t input[max_fuzzer_input_size] = { 0 };
   klee_make_symbolic(&input, sizeof(input), "input");

   size_t input_len = klee_range(0, sizeof(input), "input_len");

   LLVMFuzzerTestOneInput(input, input_len);
   }

#endif

#endif
