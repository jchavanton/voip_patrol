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

## Threat model (public repo + self-hosted runner)

Four mitigations layered:

1. **First-time contributor approval** — GitHub Settings gate.
2. **Dockerized workflow steps** — the runner host sees only docker
   commands; PR-controlled code runs inside the container.
3. **`--ephemeral` runner** — the agent self-unregisters after one
   job; systemd restarts it with a fresh token for the next.
4. **Trigger filter** — only `push` to `master` and
   `pull_request` to `master`. We deliberately avoid
   `pull_request_target`, which would run against the base ref with
   repository secrets exposed.

If any of those four stops working, re-tighten before the next
contributor PR.

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

Run as a user with sudo, not as root. Replace `jchavanton`
with your login if different.

```sh
# 1. Dedicated unprivileged user for the runner.
sudo adduser --disabled-password --gecos "GitHub Actions runner" gh-runner
sudo usermod -aG docker gh-runner  # created in step 2

# 2. Install Docker.
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

# Confirm docker works without sudo (log out and back in first if adduser
# was just run so the gh-runner group membership kicks in).
sudo -iu gh-runner docker run --rm hello-world

# 3. Install the runner agent as gh-runner. The URL + token come
#    from the "New self-hosted runner" page in GitHub Settings.
#    Replace both before running. The token is single-use and
#    expires in 1 hour.
sudo -iu gh-runner bash -c '
  mkdir -p ~/actions-runner && cd ~/actions-runner
  VERSION=2.321.0   # check the GitHub "New self-hosted runner" page for the latest
  curl -o actions-runner.tar.gz -L \
    https://github.com/actions/runner/releases/download/v${VERSION}/actions-runner-linux-x64-${VERSION}.tar.gz
  tar xzf actions-runner.tar.gz

  ./config.sh \
    --url    https://github.com/jchavanton/voip_patrol \
    --token  REPLACE_WITH_TOKEN_FROM_GITHUB_UI \
    --name   ovh3 \
    --labels ovh3 \
    --work   _work \
    --ephemeral \
    --unattended \
    --replace
'

# 4. systemd service so the runner auto-restarts and survives
#    reboots. --ephemeral makes the agent exit cleanly after each
#    job; Restart=always re-launches it, and on startup it
#    re-registers if needed. Rotating tokens are handled by
#    GitHub's config.sh --ephemeral flow.
sudo bash -c 'cat > /etc/systemd/system/gh-runner.service' <<'UNIT'
[Unit]
Description=GitHub Actions runner (voip_patrol, ovh3, ephemeral)
After=network-online.target docker.service
Wants=network-online.target
Requires=docker.service

[Service]
Type=simple
User=gh-runner
WorkingDirectory=/home/gh-runner/actions-runner
ExecStart=/home/gh-runner/actions-runner/run.sh
# Give docker builds breathing room on shutdown.
KillMode=process
KillSignal=SIGTERM
TimeoutStopSec=5min
Restart=always
RestartSec=10

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

## Ephemeral token refresh — if the runner stops re-registering

`--ephemeral` lets the runner pick up exactly one job, finish, and
exit with status 0. systemd restarts `run.sh`, which internally
asks GitHub for a fresh single-use registration token using the
credentials written by `config.sh`. If that ever breaks (expired
credentials file, revoked registration, etc.):

```sh
sudo -iu gh-runner bash -c '
  cd ~/actions-runner
  ./config.sh remove --token REPLACE_WITH_REMOVAL_TOKEN
  ./config.sh --url    https://github.com/jchavanton/voip_patrol \
              --token  REPLACE_WITH_NEW_REGISTRATION_TOKEN \
              --name   ovh3 \
              --labels ovh3 \
              --work   _work \
              --ephemeral \
              --unattended \
              --replace
'
sudo systemctl restart gh-runner.service
```

Both tokens come from Settings → Actions → Runners → ovh3 (remove
token) and "New self-hosted runner" (registration token).

## Decommissioning the runner

```sh
sudo systemctl disable --now gh-runner.service
sudo rm /etc/systemd/system/gh-runner.service
sudo systemctl daemon-reload
sudo -iu gh-runner ~/actions-runner/config.sh remove \
    --token REPLACE_WITH_REMOVAL_TOKEN
# Optional: sudo deluser --remove-home gh-runner
```
