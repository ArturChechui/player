#pragma once

// Stub header for libmad MP3 decoder
// TODO: Link against precompiled libmad.a or integrate helix decoder

typedef struct {
    // MP3 frame info
} mad_frame;

typedef struct {
    // MP3 stream state
} mad_stream;

typedef struct {
    // PCM output
    int sample[2][32 * 18];
} mad_pcm;

// Stub functions
void mad_stream_init(mad_stream *stream);
void mad_frame_init(mad_frame *frame);
int mad_frame_decode(mad_frame *frame, mad_stream *stream);
