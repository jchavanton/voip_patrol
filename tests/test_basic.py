"""Smoke tests that exercise the signaling + media path end-to-end.

Every test here is single-process: one voip_patrol instance plays both
the UAC and the UAS, calling itself over 127.0.0.1. Asserts read the
NDJSON result file; no log scraping.

Known limitation of this shape: when both legs run in the same pjsua
instance and loopback via 127.0.0.1, the UAS side's jitter buffer
discards almost every received RTP frame (observed as rtp_stats
Rx discard ~= pkt). The symptom is that media-observing tests
(tone detection, in-band DTMF) see nothing even though bytes are on
the wire. Marked xfail below; the fix likely needs either two
processes (shape A) or a self-loopback workaround in voip_patrol
(distinct SSRCs per leg, or a one-frame delay between tx and rx).
"""
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


@pytest.mark.xfail(
    reason="single-process self-loopback: UAS's jitter buffer discards "
    "~100% of RTP frames (Rx discard ~= Rx pkt), so the tone detector "
    "never sees sustained energy. Needs shape A (two processes) or a "
    "self-loopback fix in voip_patrol.",
    strict=True,
)
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


@pytest.mark.xfail(
    reason="single-process self-loopback: same RTP discard issue as "
    "test_tones_detected prevents inbound RFC 2833 DTMF from reaching "
    "the receiver.",
    strict=True,
)
def test_dtmf_roundtrip(run_scenario):
    """UAC sends '123*', UAS reports dtmf_recv on the accept result."""
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
