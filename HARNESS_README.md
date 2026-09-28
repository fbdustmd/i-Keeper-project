# NetSentry AI Harness

This folder contains repository instructions for AI-assisted development.

## Recommended Repository Layout

```text
netsentry/
├── AGENTS.md
├── ARCHITECTURE.md
├── HARNESS_README.md
├── src/
├── include/
├── tests/
├── samples/
├── docs/
│   ├── PROJECT_SPEC.md
│   ├── IMPLEMENTATION_RULES.md
│   ├── TESTING.md
│   ├── EXECUTION_PLANS.md
│   └── exec-plans/
│       ├── active/
│       └── completed/
└── Makefile
```

## How to Use with an AI Coding Agent

Do not paste the full project explanation into every prompt.

A task prompt can be small:

```text
Prepare Phase 1 using docs/exec-plans/active/phase-01-capture-environment.md.
Follow AGENTS.md.
Do not implement Phase 2.
Build and test the result.
```

or:

```text
Implement only the Ethernet parser from
docs/exec-plans/reference/phases-02-to-05-packet-parsers.md.

Follow the packet bounds rules in
docs/IMPLEMENTATION_RULES.md.

After implementation, run the relevant tests and report remaining limitations.
```

## Human Review Checkpoints

The project owner should review AI work before moving forward at these points:

1. after module/API design
2. after each parser
3. after Flow Manager data structure design
4. before TCP reassembly implementation
5. after reassembly algorithm changes
6. before declaring MVP complete

The goal is not just working code.
The owner should be able to explain the code and network concepts.
