"""Shared fixtures and helpers for voip_patrol self-tests.

These tests spawn a single voip_patrol process that runs both the UAC
(<action type="call">) and the UAS (<action type="accept">) against
itself on 127.0.0.1. We render an XML scenario from a Jinja2 template
with freshly-allocated ports, run the binary, then parse the resulting
NDJSON file and return it to the test.
"""
from __future__ import annotations

import json
import os
import shutil
import socket
import subprocess
from dataclasses import dataclass
from pathlib import Path

import pytest
from jinja2 import Environment, FileSystemLoader, StrictUndefined

REPO_ROOT = Path(__file__).resolve().parent.parent
SCENARIO_DIR = Path(__file__).resolve().parent / "scenarios"


def _find_binary() -> Path:
    """Locate the voip_patrol binary.

    Priority: VOIP_PATROL_BIN env var > repo-root ./voip_patrol > build-master/.
    """
    env = os.environ.get("VOIP_PATROL_BIN")
    if env:
        p = Path(env)
        if p.is_file() and os.access(p, os.X_OK):
            return p
        pytest.fail(f"VOIP_PATROL_BIN={env} is not an executable file")
    for candidate in [
        REPO_ROOT / "voip_patrol",
        REPO_ROOT / "build-master" / "voip_patrol",
        REPO_ROOT / "build-manual" / "voip_patrol",
    ]:
        if candidate.is_file() and os.access(candidate, os.X_OK):
            return candidate
    pytest.fail(
        "voip_patrol binary not found. Build it first, or set VOIP_PATROL_BIN "
        "to the path of the binary."
    )


def _free_port() -> int:
    """Grab an ephemeral TCP/UDP port from the kernel.

    SO_REUSEADDR lets the port be rebound by voip_patrol immediately after we
    close the probe socket. This is best-effort; truly race-free port
    allocation would require pjsua to accept port=0, which it doesn't.
    """
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(("127.0.0.1", 0))
    port = s.getsockname()[1]
    s.close()
    return port


@dataclass
class ScenarioResult:
    exit_code: int
    stdout: str
    stderr: str
    results: list  # list of dicts, one per test entry in results.json
    log_text: str
    workdir: Path


@pytest.fixture
def jinja_env() -> Environment:
    return Environment(
        loader=FileSystemLoader(str(SCENARIO_DIR)),
        undefined=StrictUndefined,
        keep_trailing_newline=True,
    )


@pytest.fixture
def voip_patrol_bin() -> Path:
    return _find_binary()


@pytest.fixture
def run_scenario(tmp_path, jinja_env, voip_patrol_bin):
    """Render a scenario template, run voip_patrol, return ScenarioResult.

    The caller passes the template name under tests/scenarios/ and any
    template context. Two ports are always provided: sip_port and rtp_port.
    """

    def _run(template: str, timeout_s: float = 30.0, **context) -> ScenarioResult:
        sip_port = _free_port()
        rtp_port = _free_port()
        ctx = {
            "sip_port": sip_port,
            "rtp_port": rtp_port,
            "workdir": str(tmp_path),
            **context,
        }
        xml = jinja_env.get_template(template).render(**ctx)
        scenario_path = tmp_path / "scenario.xml"
        scenario_path.write_text(xml)

        results_path = tmp_path / "results.json"
        log_path = tmp_path / "vp.log"

        cmd = [
            str(voip_patrol_bin),
            "-c", str(scenario_path),
            "-o", str(results_path),
            "-l", str(log_path),
            "-p", str(sip_port),
            "--rtp-port", str(rtp_port),
            "--bound-addr", "127.0.0.1",
            "--ip-addr", "127.0.0.1",
            "--udp",
            "--log-level-console", "1",
            "--record-dir", str(tmp_path) + "/",
            "--graceful-shutdown",
        ]
        proc = subprocess.run(
            cmd,
            cwd=tmp_path,
            capture_output=True,
            text=True,
            timeout=timeout_s,
        )

        results = []
        if results_path.exists():
            for line in results_path.read_text().splitlines():
                line = line.strip()
                if not line:
                    continue
                results.append(json.loads(line))

        log_text = log_path.read_text() if log_path.exists() else ""
        return ScenarioResult(
            exit_code=proc.returncode,
            stdout=proc.stdout,
            stderr=proc.stderr,
            results=results,
            log_text=log_text,
            workdir=tmp_path,
        )

    return _run


def pytest_configure(config):
    # Make the voice reference wav available to tests that render file-based
    # scenarios: voip_patrol resolves paths relative to CWD, which we set to
    # tmp_path. Symlink the reference dir in there if a test asks for it.
    pass


@pytest.fixture
def voice_ref_dir(tmp_path) -> Path:
    """Symlink voice_ref_files/ into the per-test tmp_path.

    voip_patrol opens the wav by relative path, which in run_scenario is
    resolved against cwd=tmp_path. Only create this when a test actually
    needs file playback.
    """
    src = REPO_ROOT / "voice_ref_files"
    dst = tmp_path / "voice_ref_files"
    if not src.is_dir():
        pytest.skip(f"voice_ref_files/ not present at {src}")
    if not dst.exists():
        os.symlink(src, dst)
    return dst
