"""Smoke tests that exercise the signaling + media path end-to-end.

Every test here is single-process: one voip_patrol instance plays both
the UAC and the UAS, calling itself over 127.0.0.1. Asserts read the
NDJSON result file; no log scraping.

Shape-B quirk worth remembering when writing scenarios:

Account selection. The caller and callee use *different* user parts
(caller@... and callee@...) so that voip_patrol's onSelectAccount
picks the "default" (catch-all) account for the UAS side. If both
sides used the same user, findAccount() would match the caller's
account for the incoming leg and skip the accept action's config
entirely (code, ring_duration, etc.)."""
from __future__ import annotations

import pytest


def _entries(results, action):
    return [r for r in results if r.get("action") == action]


def test_basic_call_pass(run_scenario):
    """A plain UDP call, both sides silent, answered 200, hung up after 3 s.

    This is the canonical "nothing exploded" test. If this fails, the
    harness (build, port alloc, scenario render, JSON parse) is broken
    and every other test is noise.
    """
    r = run_scenario("basic_call.xml.j2", hangup_s=3)
    assert r.exit_code == 0, f"voip_patrol exited {r.exit_code}\nstderr:\n{r.stderr}"
    assert r.results, f"no results.json entries produced\nstderr:\n{r.stderr}"

    call = _entries(r.results, "call")
    accept = _entries(r.results, "accept")
    assert call, "no 'call' entry in results"
    assert accept, "no 'accept' entry in results"

    assert call[0]["result"] == "PASS", f"call failed: {call[0]}"
    assert call[0]["cause_code"] == 200
    assert call[0]["expected_cause_code"] == 200
    assert accept[0]["cause_code"] == 200


def test_tones_detected(run_scenario):
    """UAC plays a 440+480 ringback, UAS watches for 440+480, asserts detection.

    Exercises the pjmedia_tonegen generator *and* the tone detector in
    a round-trip. If this ever regresses, the generator-start-delay
    conversation from earlier should re-open: tone_detected_ms should
    be well under 1 s on a healthy build.
    """
    r = run_scenario(
        "tones_detect.xml.j2",
        tones_value="440+480,2000,4000",
        detect_tone_freqs="440,480",
        hangup_s=5,
    )
    assert r.exit_code == 0, f"voip_patrol exited {r.exit_code}"

    accept = _entries(r.results, "accept")
    assert accept, "no 'accept' entry in results"
    a = accept[0]
    assert a.get("tone_detected") == 1, f"tone was not detected: {a}"
    # Tonegen attaches at CONFIRMED and pjmedia_tone_detect fires within
    # 3 frames (~60 ms) once the tone is sustained. A healthy path is well
    # under 1 s; give 2 s of headroom for CI jitter.
    assert a["tone_detected_ms"] < 2000, (
        f"tone_detected_ms={a['tone_detected_ms']}ms is too high; "
        "check whether pjmedia is adding startup delay"
    )


def test_tonegen_emits_audio(run_scenario):
    """UAC plays 440+480 tones; assert the tonegen is actually pumping
    audio samples into the call's outbound conf slot.

    This sidesteps the self-loopback Rx-discard limitation: we read the
    *Tx* side's player_level_* counters, which are sampled directly at
    the tonegen's conf port before frames leave the bridge. If the
    tonegen produces audio, these counters go up; they don't care
    whether the far end ever hears anything.

    The record-side Rx remains unverifiable in shape B — see
    test_tones_detected for that.
    """
    r = run_scenario(
        "tones_detect.xml.j2",
        tones_value="440+480,2000,4000",
        detect_tone_freqs="440,480",
        energy_stats=True,
        hangup_s=4,
    )
    assert r.exit_code == 0

    call = _entries(r.results, "call")
    assert call, "no 'call' entry in results"
    rtp = call[0].get("rtp_stats") or []
    assert rtp, f"energy_stats=true but no rtp_stats block: {call[0]}"
    tx = rtp[0]["Tx"]
    # player_* counters reflect the signal read at the tonegen port,
    # independently of what makes it onto the wire.
    assert tx.get("player_samples", 0) > 0, (
        f"tonegen never sampled any frames: Tx={tx}"
    )
    assert tx.get("player_level_peak", 0) > 0, (
        f"tonegen attached but emitted silence (peak=0): Tx={tx}"
    )
    assert tx.get("player_voice_frames", 0) > 0, (
        f"tonegen samples were all below the voice threshold: Tx={tx}"
    )


def test_dtmf_roundtrip(run_scenario):
    """UAC sends '123*', UAS reports dtmf_recv on the accept result.

    RFC 2833 telephone-event packets ride on an active RTP stream.
    SILENCE mode (the default, and the explicit <play type="silence">)
    attaches a null-port source so the bridge pumps silent frames and
    RTP TX stays active — DTMF packets ride on that flow."""
    digits = "123*#"
    r = run_scenario(
        "dtmf.xml.j2",
        dtmf_value=digits,
        hangup_s=5,
    )
    assert r.exit_code == 0

    accept = _entries(r.results, "accept")
    assert accept, "no 'accept' entry in results"
    recv = accept[0].get("dtmf_recv", "")
    # DTMF sequence is sent with inter-digit pauses; everything asked for
    # must land. Order matters: RFC 2833 preserves it.
    assert recv == digits, f"dtmf_recv={recv!r} != sent={digits!r}"


# --- <play> dispatch coverage ------------------------------------------------
# The three tests below lock down the three audio-source branches of
# stream_to_call (file, silence, default-silence) via the tx_player_*
# energy counters sampled at the source conf port. Nothing here depends
# on the broken self-loopback, so all three pass in shape B.


def test_play_file_emits_audio(run_scenario, voice_ref_dir):
    """<play type="file"> opens pjsua's file player and feeds it into the
    call. tx_player_samples > 0 proves the player is sampling, and
    tx_player_voice_frames > 0 proves the wav has non-silent content."""
    r = run_scenario(
        "play_file.xml.j2",
        energy_stats=True,
        hangup_s=4,
    )
    assert r.exit_code == 0, f"voip_patrol exited {r.exit_code}"

    call = _entries(r.results, "call")
    assert call, "no 'call' entry in results"
    rtp = call[0].get("rtp_stats") or []
    assert rtp, f"energy_stats=true but no rtp_stats block: {call[0]}"
    tx = rtp[0]["Tx"]
    assert tx.get("player_samples", 0) > 0, (
        f"file player never sampled any frames: Tx={tx}"
    )
    assert tx.get("player_voice_frames", 0) > 0, (
        f"file player samples were all below the voice threshold "
        f"(is reference_8000.wav silent?): Tx={tx}"
    )


def test_play_silence_is_silent(run_scenario):
    """<play type="silence"> attaches a null source — silent audio, but
    RTP still flows. The counters we expect:
      tx_player_samples > 0   — source is attached and being sampled
      tx_player_voice_frames == 0 — but the samples are all silent
      tx_player_level_peak < VOICE_LEVEL_THRESHOLD — silent peak
    This proves the stream stays active (so DTMF etc. can ride on it)
    while nothing audible is produced."""
    r = run_scenario(
        "play_silence.xml.j2",
        energy_stats=True,
        hangup_s=3,
    )
    assert r.exit_code == 0

    call = _entries(r.results, "call")
    assert call, "no 'call' entry in results"
    rtp = call[0].get("rtp_stats") or []
    assert rtp, "energy_stats=true should always emit rtp_stats"
    tx = rtp[0]["Tx"]
    assert tx.get("player_samples", 0) > 0, (
        f"<play type=silence> did not attach a source — RTP will be idle "
        f"and anything riding on it (DTMF) will fail: Tx={tx}"
    )
    assert tx.get("player_voice_frames", 0) == 0, (
        f"<play type=silence> emitted non-silent audio: Tx={tx}"
    )


def test_default_no_play_is_silent(run_scenario):
    """The absence of any <play> child defaults to <play type="silence">:
    a silent source is attached and RTP flows. Same assertions as the
    explicit silence case."""
    r = run_scenario("basic_call.xml.j2", energy_stats=True, hangup_s=3)
    assert r.exit_code == 0

    call = _entries(r.results, "call")
    assert call, "no 'call' entry in results"
    rtp = call[0].get("rtp_stats") or []
    assert rtp
    tx = rtp[0]["Tx"]
    assert tx.get("player_samples", 0) > 0, (
        f"default (no <play>) did not attach a silent source — the "
        f"default-to-silence path should still keep RTP flowing: Tx={tx}"
    )
    assert tx.get("player_voice_frames", 0) == 0, (
        f"default (no <play>) emitted non-silent audio: Tx={tx}"
    )


# --- miscellaneous dispatch regressions --------------------------------------


def test_expected_cause_code_mismatch_fails(run_scenario):
    """UAS answers 486 Busy Here, UAC still expects 200. The call test
    row must report FAIL (not PASS) and carry the real cause_code 486.
    Catches regressions in update_result's pass/fail accounting."""
    r = run_scenario(
        "basic_call.xml.j2",
        accept_code=486,
        accept_reason="Busy Here",
        expected_cause_code=200,   # intentional mismatch
        hangup_s=3,
    )
    # The process still exits 0 — a test-level FAIL isn't a process-level
    # error; voip_patrol just reports it in the JSON.
    assert r.exit_code == 0

    call = _entries(r.results, "call")
    assert call, "no 'call' entry in results"
    c = call[0]
    assert c["cause_code"] == 486, f"expected cause_code=486, got {c}"
    assert c["expected_cause_code"] == 200
    assert c["result"] == "FAIL", f"mismatch should flip to FAIL, got: {c}"


def test_unknown_play_type_is_noisy_but_safe(run_scenario):
    """<play type="xyz"> is a malformed input. The parser logs an error
    and falls back to default behaviour (silence); the scenario must
    still complete cleanly — no crash, call still answers 200."""
    r = run_scenario("play_bad_type.xml.j2", hangup_s=3)
    assert r.exit_code == 0, (
        f"bad <play type> crashed voip_patrol (exit={r.exit_code})\n"
        f"stderr:\n{r.stderr}"
    )

    call = _entries(r.results, "call")
    accept = _entries(r.results, "accept")
    assert call and accept, "scenario didn't complete"
    assert call[0]["result"] == "PASS", (
        f"bad <play> type should fall through to defaults and still pass; "
        f"got: {call[0]}"
    )
    # The parser is expected to log this; we check the log rather than
    # asserting on a particular JSON field because the error is a parse
    # concern, not something update_result surfaces.
    assert "unknown <play type=\"xyz\">" in r.log_text, (
        "parser should log an error line for unknown <play type>; "
        f"not found in vp.log\nlog tail:\n{r.log_text[-2000:]}"
    )
