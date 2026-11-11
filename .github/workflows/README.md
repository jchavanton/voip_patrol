# CI setup

GitHub Actions workflows live under `.github/workflows/`.

## `tests.yml`

Builds `tests/Dockerfile.test` and runs the pytest suite inside the
resulting container. Triggers on push to `master` and on
`pull_request` targeting `master`. Fork PRs appear via
`pull_request` and are gated by the repository's
"Require approval for first-time contributors" setting — see the
one-time GitHub UI step below.

The job pins to the `ovh3` self-hosted runner. Every step runs
inside Docker on that host, so the runner user only ever executes
`docker build` + `docker run` — PR code is isolated inside the
disposable container.

## `release.yml`

Triggers when a bare semver tag (e.g. `1.0.0`) is pushed. Matches
the pattern `[0-9]+.[0-9]+.[0-9]+` — no leading `v`, consistent
with the existing tag history. Runs on the same `ovh3` self-hosted
runner and reuses the `tests/Dockerfile.test` pipeline: tests must
pass before the release is published. On success, `gh release
create` posts a GitHub Release with auto-generated notes from the
commits since the previous tag. Needs `permissions: contents:
write` so the default `GITHUB_TOKEN` can publish.

Releases are cut with [`bumpversion`](https://github.com/c4urself/bump2version),
configured in `.bumpversion.cfg` at the repo root. It bumps the
version string in `include/version.h`, `docker/Dockerfile`, and
`docker/build_image.sh`, makes a `release: bump VERSION to …`
commit, and tags it with the bare `{new_version}` — exactly the
shape this workflow filters on.

```sh
bumpversion patch   # or minor / major
git push --follow-tags
```

The tag push triggers `release.yml`; tests run on ovh3, and on
success the Release lands in GitHub with auto-generated notes.

## Threat model (public repo + self-hosted runner)

Three mitigations layered:

1. **First-time contributor approval** — GitHub Settings gate. Fork
   PRs from new contributors won't run the workflow until a
   maintainer approves.
2. **Dockerized workflow steps** — the runner host sees only docker
   commands; PR-controlled code runs inside the disposable
   container. The container has no bind-mounts into the host and no
   injected secrets.
3. **Trigger filter** — only `push` to `master` and `pull_request`
   to `master`. We deliberately avoid `pull_request_target`, which
   would run against the base ref with repository secrets exposed
   to fork code.

> An earlier revision added a fourth layer (`--ephemeral` runner,
> self-unregisters after each job). That turned out not to interact
> cleanly with systemd `Restart=always` — once credentials were
> deleted by the ephemeral exit, the restart loop couldn't
> re-register itself without a new one-shot token on every bounce.
> We now run the agent in **persistent** mode instead: credentials
> stay on disk, the agent reconnects between jobs. Re-tightening
> with ephemeral would need an `ExecStartPre` script that mints a
> fresh registration token via a GitHub PAT — a credential the
> ephemeral setup was supposed to avoid in the first place. Not
> worth the trade. If this ever becomes necessary, the docs for
> that path are at <https://docs.github.com/en/actions/hosting-your-own-runners/managing-self-hosted-runners/autoscaling-with-self-hosted-runners>.

## One-time GitHub UI configuration

Settings → Actions → General:

- "Fork pull request workflows from outside collaborators" →
  **Require approval for first-time contributors** (minimum) or
  **Require approval for all outside collaborators** (stricter).
- "Actions permissions" → "Allow enterprise, and select
  non-enterprise, actions and reusable workflows" or looser; we
  only use `actions/checkout`.

Settings → Actions → Runners → **New self-hosted runner** → **Linux
x64**. Note the URL and registration token shown on that page;
you'll paste both into `./config.sh` below. Tokens expire after
one hour and are single-use.

## One-time ovh3 host setup

Run as a user with sudo, not as root.

```sh
# 1. Dedicated unprivileged user for the runner.
sudo adduser --disabled-password --gecos "GitHub Actions runner" gh-runner

# 2. Install Docker from docker.com's apt repo.
sudo apt-get update
sudo apt-get install -y ca-certificates curl
sudo install -m 0755 -d /etc/apt/keyrings
sudo curl -fsSL https://download.docker.com/linux/debian/gpg \
    -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc
echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] \
    https://download.docker.com/linux/debian $(. /etc/os-release && echo $VERSION_CODENAME) stable" \
    | sudo tee /etc/apt/sources.list.d/docker.list
sudo apt-get update
sudo apt-get install -y docker-ce docker-ce-cli containerd.io \
    docker-buildx-plugin docker-compose-plugin
sudo usermod -aG docker gh-runner

# Confirm Docker works as gh-runner (no sudo needed):
sudo -iu gh-runner docker run --rm hello-world

# 3. Install the runner agent as gh-runner. The URL + token come
#    from the "New self-hosted runner" page in GitHub Settings.
#    The token is single-use and expires in 1 hour. No --ephemeral:
#    credentials persist across restarts so systemd Restart=always
#    actually works.
sudo -iu gh-runner bash -c '
  mkdir -p ~/actions-runner && cd ~/actions-runner
  VERSION=2.338.2   # check the GitHub "New self-hosted runner" page for the latest;
                    # GitHub enforces a minimum (currently >= 2.329.0) and will
                    # refuse registration from older agents with a 404.
  curl -o actions-runner.tar.gz -L \
    https://github.com/actions/runner/releases/download/v${VERSION}/actions-runner-linux-x64-${VERSION}.tar.gz
  tar xzf actions-runner.tar.gz

  ./config.sh \
    --url    https://github.com/jchavanton/voip_patrol \
    --token  REPLACE_WITH_TOKEN_FROM_GITHUB_UI \
    --name   ovh3 \
    --labels ovh3 \
    --work   _work \
    --unattended \
    --replace
'

# 4. systemd service. Restart=always brings the agent back if it
#    crashes or GitHub drops the connection; persistent credentials
#    mean no token is needed at restart time. TimeoutStopSec covers
#    an in-flight docker build + pytest.
sudo bash -c 'cat > /etc/systemd/system/gh-runner.service' <<'UNIT'
[Unit]
Description=GitHub Actions runner (voip_patrol, ovh3)
After=network-online.target docker.service
Wants=network-online.target
Requires=docker.service

[Service]
Type=simple
User=gh-runner
WorkingDirectory=/home/gh-runner/actions-runner
ExecStart=/home/gh-runner/actions-runner/run.sh
Restart=always
RestartSec=10
TimeoutStopSec=15min

[Install]
WantedBy=multi-user.target
UNIT
sudo systemctl daemon-reload
sudo systemctl enable --now gh-runner.service
sudo systemctl status gh-runner.service --no-pager
```

## Verifying the runner is live

Back in GitHub → Settings → Actions → Runners you should see
`ovh3` with a green "Idle" dot. Trigger a run with:

```sh
git commit --allow-empty -m "ci: smoke-test the ovh3 runner"
git push
```

Then watch the run under the Actions tab. First build takes
~3-5 min (pjproject compile); cached rebuilds run in well under a
minute once the layer cache warms up.

## Rotating the agent's registration

The agent uses the credentials written by `config.sh` at install time
and needs no token for day-to-day operation. You only need new tokens
to re-register — for example if the credentials file is lost, the
runner entry on GitHub is manually revoked, or the agent is being
moved to a different repo.

```sh
sudo systemctl stop gh-runner.service
sudo -iu gh-runner bash -c '
  cd ~/actions-runner
  ./config.sh remove --token REPLACE_WITH_REMOVAL_TOKEN
  ./config.sh --url    https://github.com/jchavanton/voip_patrol \
              --token  REPLACE_WITH_NEW_REGISTRATION_TOKEN \
              --name   ovh3 \
              --labels ovh3 \
              --work   _work \
              --unattended \
              --replace
'
sudo systemctl start gh-runner.service
```

Both tokens come from Settings → Actions → Runners → ovh3 → **Remove
runner** (removal token) and **New self-hosted runner** (registration
token). If the local `.credentials` file is already gone, the
`config.sh remove` step prints "Cannot connect to server … Skipping"
— that's fine, continue to the register step.

## Decommissioning the runner

```sh
sudo systemctl disable --now gh-runner.service
sudo rm /etc/systemd/system/gh-runner.service
sudo systemctl daemon-reload
sudo -iu gh-runner ~/actions-runner/config.sh remove \
    --token REPLACE_WITH_REMOVAL_TOKEN
# Optional: sudo deluser --remove-home gh-runner
```
