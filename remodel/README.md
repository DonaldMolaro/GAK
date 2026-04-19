# remodel deprecation

The `remodel` subtree is deprecated and not part of the supported build.

Why:

- It depended on an external `Makeconf` that is not included in this repository.
- It was written against an older version of the GA API and no longer matches the modernized library surface.
- It has not been migrated or covered by the current test/build flow.

Current status:

- `make -C remodel` intentionally fails with a deprecation message.
- The supported entry points are the library in `src/`, the examples in `examples/`, and the test suites in `tests/` and `examples/test_main.cc`.

If we want `remodel` back in the future, it should be treated as a fresh migration project rather than a small compatibility fix.
