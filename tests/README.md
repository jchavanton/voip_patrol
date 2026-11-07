# voip_patrol self-tests

A small pytest harness that spawns voip_patrol against itself: one process
runs both the UAC (`<action type="call">`) and the UAS
(`<action type="accept">`) on 127.0.0.1, so the test needs no external
peer.

## Prereqs

* A built `voip_patrol` binary (the harness looks at
  `$VOIP_PATROL_BIN`, then `./voip_patrol`, then
  `./build-master/voip_patrol`, then `./build-manual/voip_patrol`).
* **Either** Docker (recommended — no Python on your host)
  **or** Python ≥ 3.9.

## Run in a container (recommended)

From the repo root:

```sh
docker build -f tests/Dockerfile.test -t voip_patrol-tests .
docker run --rm voip_patrol-tests
```

The image is `python:3.12-slim` + the runtime libs voip_patrol links
against (libasound2, libopus0, libssl3, libcurl4) + pytest + Jinja2.
It copies the pre-built `./voip_patrol` binary in and runs `pytest`
against it. No host Python changes.

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
