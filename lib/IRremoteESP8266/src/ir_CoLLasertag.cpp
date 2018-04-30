// Copyright 2017 David Conran

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtimer.h"
#include "IRutils.h"

// "Call of Life" style
//   LL        AAA    SSSSS  EEEEEEE RRRRRR  TTTTTTT   AAA     GGGG
//   LL       AAAAA  SS      EE      RR   RR   TTT    AAAAA   GG  GG
//   LL      AA   AA  SSSSS  EEEEE   RRRRRR    TTT   AA   AA GG
//   LL      AAAAAAA      SS EE      RR  RR    TTT   AAAAAAA GG   GG
//   LLLLLLL AA   AA  SSSSS  EEEEEEE RR   RR   TTT   AA   AA  GGGGGG

// Constants
#define MIN_COL_LASERTAG_SAMPLES                42U
#define COL_LASERTAG_TICK                      420U
#define COL_LASERTAG_HEADER                   1600U
#define COL_LASERTAG_MIN_GAP               100000UL  // Completely made up amount.
#define COL_LASERTAG_ONE_TICK    (COL_LASERTAG_TICK * 2U)
#define COL_LASERTAG_ZERO_TICK   (COL_LASERTAG_TICK)
#define COL_LASERTAG_TOLERANCE                  15U   // Use instead of EXCESS and TOLERANCE.
#define COL_LASERTAG_DELTA                     150U   // Use instead of EXCESS and TOLERANCE.

const int16_t kSPACE = 1;
const int16_t kMARK = 0;

#if SEND_COL_LASERTAG
// Send a Lasertag packet.
//
// Args:
//   data:    The message you wish to send.
//   nbits:   Bit size of the protocol you want to send.
//   repeat:  Nr. of extra times the data will be sent.
//
// Status: Not yet working
//

void IRsend::sendCoLLasertag(uint64_t data, uint16_t nbits, uint16_t repeat) {
  if (nbits > sizeof(data) * 8)
    return;  // We can't send something that big.

  // Set 36kHz IR carrier frequency & a 1/4 (25%) duty cycle.
  // NOTE: duty cycle is not confirmed. Just guessing based on RC5/6 protocols.
  enableIROut(36, 25);

  for (uint16_t i = 0; i <= repeat; i++) {
    // Data
    for (uint64_t mask = 1ULL << (nbits - 1); mask; mask >>= 1)
      if (data & mask) {  // 1
        space(LASERTAG_TICK);  // 1 is space, then mark.
        mark(LASERTAG_TICK);
      } else {  // 0
        mark(LASERTAG_TICK);  // 0 is mark, then space.
        space(LASERTAG_TICK);
      }
    // Footer
    space(LASERTAG_MIN_GAP);
  }
}
#endif  // SEND_LASERTAG

#if DECODE_COL_LASERTAG
// Decode the supplied Lasertag message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA / Appears to be working 90% of the time.
//
// Ref:
//   http://www.sbprojects.com/knowledge/ir/rc5.php
//   https://en.wikipedia.org/wiki/RC-5
//   https://en.wikipedia.org/wiki/Manchester_code
bool IRrecv::decodeCoLLasertag(decode_results *results, uint16_t nbits,
                            bool strict) {

  if (results->rawlen < MIN_COL_LASERTAG_SAMPLES) return false;

  // Compliance
  if (strict && nbits != COL_LASERTAG_BITS) return false;
  uint16_t offset = OFFSET_START;
  uint16_t used = 0;
  uint64_t data = 0;
  uint16_t actual_bits = 0;

  // Header
  if (!match(results->rawbuf[offset++], COL_LASERTAG_HEADER,0,COL_LASERTAG_DELTA)) return false;

  // Data
	for (; offset < results->rawlen; actual_bits++) {
		if(match(results->rawbuf[offset], COL_LASERTAG_ONE_TICK, COL_LASERTAG_TOLERANCE))
			data = (data << 1) | 1;  // 1
		else
			data <<= 1;  // 0

		offset++;
	}

	// There is a footer or checksum, but I haven't figured it out yet.

  // Success
  results->decode_type = COL_LASERTAG;
  results->value = data;
  results->address = (data >> (8*2)) & 0xff; // Team
  results->command = (data >> 8) & 0xff;   // Damage
  results->repeat = false;
  results->bits = actual_bits;
  return true;
}
#endif  // DECODE_LASERTAG
