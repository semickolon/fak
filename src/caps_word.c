#include "caps_word.h"

#include "ch552.h"

__bit caps_word_state = 0;

void caps_word_toggle() {
    caps_word_state = !caps_word_state;
}

__bit caps_word_active() {
    return caps_word_state;
}

__bit caps_word_handle_key(uint8_t code) {
  if (caps_word_state == 0) {
    return 0;
  }

  // this assumes US layout on the host OS.
  if ((code >= 0x04) && code < (0x04 + 26)) {
    // A..Z
    return 1;
  } else if ((code >= 0x04 + 26) && code < (0x04 + 26 + 10)) { // 1..0
  } else if (code == 0x2A) { // backspace
  } else if (code == 0x2D) { // minus
    return 1;
  } else if (code == 0x4C) { // delete
  } else {
    // otherwise: not an accepted key; disable caps word
    caps_word_state = 0;
  }

  return 0;
}
