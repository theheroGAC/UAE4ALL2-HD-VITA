/*
 * midi_synth.h - Amiga serial MIDI / MT-32 style synthesizer for UAE4All2
 *
 * Captures MIDI bytes written by Amiga software to the serial port
 * (SERDAT $DFF030 and CIAA SDR) and renders them with a built-in
 * 32-voice polyphonic synthesizer mixed into the Paula audio output.
 */

#ifndef UAE_MIDI_SYNTH_H
#define UAE_MIDI_SYNTH_H

#include <stdint.h>

#if defined(__PSP2__) || defined(__SWITCH__)

#ifdef __cplusplus
extern "C" {
#endif

/* One-time setup (no-op, kept for symmetry). */
void midi_synth_init(void);

/* Reset all voices, parser state and input ring (call on emulation reset). */
void midi_synth_reset(void);

/* Enable / disable the synthesizer. Disabling silences all voices. */
void midi_synth_set_enabled(int enabled);
int  midi_synth_enabled(void);

/* Immediately silence all active notes (call when entering the GUI). */
void midi_synth_all_notes_off(void);

/* Feed one MIDI byte captured from the emulated serial hardware. */
void midi_synth_feed_byte(uint8_t byte);

/* Mix synth output into an existing stereo/mono sample buffer.
 * frames = number of output frames, channels = 1 or 2. */
void midi_synth_mix(int16_t *samples, int frames, int channels, int output_rate);

#ifdef __cplusplus
}
#endif

#endif /* __PSP2__ || __SWITCH__ */

#endif /* UAE_MIDI_SYNTH_H */
