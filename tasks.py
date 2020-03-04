from invoke import run, task

@task()
def version(ctx, patch=False, minor=False, major=False):
    if major:
        level = "major"
    elif minor:
        level = "minor"
    elif patch:
        level = "patch"
    else:
        print('Must specify one of --patch, --minor, or --major')
        sys.exit(1)
    ctx.run("bumpversion {level} --allow-dirty".format(level=level))
