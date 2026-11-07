# voip_patrol self-tests

A small pytest harness that spawns voip_patrol against itself: one process
runs both the UAC (`<action type="call">`) and the UAS
(`<action type="accept">`) on 127.0.0.1, so the test needs no external
peer.

## Prereqs

* Docker — no host build, no host Python needed. **or**
* A built `voip_patrol` binary + Python ≥ 3.9 (the harness looks at
  `$VOIP_PATROL_BIN`, then `./voip_patrol`, then
  `./build-master/voip_patrol`, then `./build-manual/voip_patrol`).

## Run in a container (recommended)

From the repo root:

```sh
docker build -f tests/Dockerfile.test -t voip_patrol-tests .
docker run --rm voip_patrol-tests
```

`tests/Dockerfile.test` is a two-stage build. Stage 1 compiles
pjproject + voip_patrol from the in-tree source (same recipe as
`docker/Dockerfile`). Stage 2 is `python:3.12-slim-trixie` with the
runtime libs voip_patrol links against (libasound2, libopus0, libssl3,
libcurl4) + pytest + Jinja2. No host changes.

First build takes a few minutes (pjproject configure + build);
subsequent builds reuse the cached layers.

## Run on the host

```sh
pip install -r tests/requirements.txt
pytest tests/ -v
```

Or with a specific binary path:

```sh
VOIP_PATROL_BIN=/path/to/voip_patrol pytest tests/ -v
```

## Add a test

1. Drop a Jinja2 scenario template under `tests/scenarios/` (extend
   `basic_call.xml.j2` if you only need to change the `<play>` blocks).
2. Add a test function in `tests/test_*.py` that calls
   `run_scenario("your_template.xml.j2", **context)` and asserts on the
   returned `ScenarioResult.results` list (one dict per
   `<action type="accept|call">` entry).

Every run gets fresh ephemeral SIP and RTP ports, a per-test tmp
workdir, and its own `results.json` / `vp.log`.
