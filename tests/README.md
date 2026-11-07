# Lifecycle regression tests

Run `./tests/run.sh` from the repository root. Only a C11 compiler is required.
The test includes the production filter implementation and substitutes a minimal
OBS API double. The double deliberately delays completion of output stop, rejects
closing attached streams, and rejects destroying mapped staging surfaces.

Coverage includes initial tick/render, dimension changes before and after tick,
multiple ticks during asynchronous stop, disable/re-enable during stop, disabled
cleanup, open/start failure and retry, zero dimensions, a missing target, and
both active and stopping destruction.

Optional sanitizer check:

```sh
CFLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer -g' ./tests/run.sh
```

These checks do not replace a native OBS build or a real virtual-camera test.
In OBS, also verify startup, resizing an enabled source, toggling the filter,
and removing it while capture is active. The asynchronous stop model follows
OBS 26.0.2 `libobs/obs-output.c`: raw video disconnects before `active` clears,
and starting an output waits for its stopping event.
