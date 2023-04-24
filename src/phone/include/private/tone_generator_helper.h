//
// Created by Oliver Epper on 17.04.23.
//

#ifndef PHONE_TONE_GENERATOR_HELPER_H
#define PHONE_TONE_GENERATOR_HELPER_H

#include <pjsua2.hpp>

pj::ToneDescVector call_waiting_sequence() {
    pj::ToneDescVector sequence{};
    pj::ToneDesc tone;
    tone.freq1 = 466;
    tone.freq2 = 932;
    tone.on_msec = 500;
    tone.off_msec = 100;
    tone.volume = 5000;
    sequence.push_back(tone);
    tone.off_msec = 4000;
    sequence.push_back(tone);
    return sequence;
}

void play_tones(pj::ToneGenerator &generator, const std::string &digits) {
    pj::ToneDigitVector tone_digits;
    for (const char c : digits) {
        pj::ToneDigit tone_digit;
        tone_digit.digit = c;
        tone_digit.on_msec = 100;
        tone_digit.off_msec = 20;
        tone_digit.volume = 5000;
        tone_digits.push_back(tone_digit);
    }
    generator.playDigits(tone_digits);
}

#endif //PHONE_TONE_GENERATOR_HELPER_H
